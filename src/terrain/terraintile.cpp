#include "terraintile.h"

#include <cmath>

#include <glow/Texture.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include <glm/glm.hpp>

#include "pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxRigidStatic.h>
#include <PxShape.h>
#include <PxMaterial.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>

#include "terrain.h"
#include "elements.h"

TerrainTile::TerrainTile(Terrain & terrain, const TileID & tileID)
: m_tileID(tileID)
, m_terrain(terrain)
, m_heightTex(nullptr)
, m_heightBuffer(nullptr)
, m_program(nullptr)
, m_heightField(nullptr)
{
    terrain.registerTile(tileID, *this);

    // compute position depending on TileID, which sets the row/column positions of the tile
    float minX = terrain.settings.tileSizeX() * (tileID.x - 0.5f);
    float minZ = terrain.settings.tileSizeZ() * (tileID.z - 0.5f);
    m_transform = glm::mat4(
        terrain.settings.intervalX(), 0, 0, 0,
        0, 1, 0, 0,
        0, 0, terrain.settings.intervalZ(), 0,
        minX, 0, minZ, 1);
}

TerrainTile::~TerrainTile()
{
    delete m_heightField;
}

void TerrainTile::bind(const glowutils::Camera & camera)
{
    if (!m_program)
        initializeProgram();
    if (!m_heightTex)
        initialize();
    if (!m_bufferUpdateList.empty())
        updateGlBuffers();

    assert(m_program);
    assert(m_heightField);
    assert(m_heightTex);

    glActiveTexture(GL_TEXTURE0 + 0);
    m_heightTex->bind();

    m_program->use();
    m_program->setUniform("cameraposition", camera.eye());
    glm::mat4 modelView = camera.view() * m_transform;
    m_program->setUniform("modelView", modelView);
    glm::mat4 modelViewProjection = camera.viewProjection() * m_transform;
    m_program->setUniform("modelViewProjection", modelViewProjection);

    m_terrain.setUpLighting(*m_program);

}

void TerrainTile::unbind()
{
    m_program->release();

    m_heightTex->unbind();
}

void TerrainTile::initialize()
{
    assert(m_heightField);

    m_heightBuffer = new glow::Buffer(GL_TEXTURE_BUFFER);
    m_heightBuffer->setData(*m_heightField, GL_DYNAMIC_DRAW);

    m_heightTex = new glow::Texture(GL_TEXTURE_BUFFER);
    m_heightTex->bind();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_heightBuffer->id());
    m_heightBuffer->unbind();

    m_program->setUniform("modelTransform", m_transform);
    m_program->setUniform("heightField", 0);
    m_program->setUniform("tileRowsColumns", glm::uvec2(m_terrain.settings.rows, m_terrain.settings.columns));
}

using namespace physx;

PxShape * TerrainTile::pxShape() const
{
    assert(m_pxShape);
    return m_pxShape;
}

void TerrainTile::createPxObjects(PxRigidStatic & pxActor)
{
    const unsigned int numSamples = m_terrain.settings.rows * m_terrain.settings.columns;

    // scale height so that we use the full range of PxI16=short
    PxReal heightScaleToWorld = m_terrain.settings.maxHeight / std::numeric_limits<PxI16>::max();
    assert(heightScaleToWorld >= PX_MIN_HEIGHTFIELD_Y_SCALE);
    float heightScaleToPx = std::numeric_limits<PxI16>::max() / m_terrain.settings.maxHeight;

    PxHeightFieldSample * hfSamples = new PxHeightFieldSample[numSamples];
    PxMaterial ** materials = nullptr;

    pxSamplesAndMaterials(hfSamples, heightScaleToPx, materials);

    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbRows = m_terrain.settings.rows;
    hfDesc.nbColumns = m_terrain.settings.columns;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof(PxHeightFieldSample);

    PxHeightField * pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    assert(m_terrain.settings.intervalX() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    assert(m_terrain.settings.intervalZ() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    // create height field geometry and set scale
    PxHeightFieldGeometry pxHfGeometry(pxHeightField, PxMeshGeometryFlags(),
        heightScaleToWorld, m_terrain.settings.intervalX(), m_terrain.settings.intervalZ());
    m_pxShape = pxActor.createShape(pxHfGeometry, materials, 1);

    assert(m_pxShape);

    delete[] hfSamples;
    delete[] materials;
}

float TerrainTile::heightAt(unsigned int row, unsigned int column) const
{
    assert(m_heightField);
    return m_heightField->at(column + row * m_terrain.settings.columns);
}

bool TerrainTile::heightAt(unsigned int row, unsigned int column, float & height) const
{
    if (row >= m_terrain.settings.rows || column >= m_terrain.settings.columns)
        return false;

    height = heightAt(row, column);
    return true;
}

void TerrainTile::setHeight(unsigned int row, unsigned int column, float value)
{
    assert(m_heightField);
    m_heightField->at(column + row * m_terrain.settings.columns) = value;
}

// mostly from OpenSceneGraph: osgTerrain/Layer
float TerrainTile::interpolatedHeightAt(float normX, float normZ) const
{
    double row_r, column_r;
    double rowFraction = std::modf(normX * m_terrain.settings.rows, &row_r);
    double columnFraction = std::modf(normZ * m_terrain.settings.columns, &column_r);

    unsigned int row = static_cast<unsigned int>(row_r);
    unsigned int column = static_cast<unsigned int>(column_r);

    double value = 0.0;
    double div = 0.0;
    float gridValue;
    double mix;

    mix = (1.0 - rowFraction) * (1.0 - columnFraction);
    if (mix > 0.0 && heightAt(row, column, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = (rowFraction) * (1.0 - columnFraction);
    if (mix > 0.0 && heightAt(row + 1, column, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = rowFraction * columnFraction;
    if (mix > 0.0 && heightAt(row + 1, column + 1, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = (1.0 - rowFraction) * columnFraction;
    if (mix > 0.0 && heightAt(row, column + 1, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    if (div != 0.0)
    {
        return static_cast<float>(value / div);
    }

    return 0.0f;
}

void TerrainTile::addBufferUpdateRange(GLintptr offset, GLsizeiptr length)
{
    m_bufferUpdateList.push_front(std::pair<GLintptr, GLsizeiptr>(offset, length));
}

void TerrainTile::updateGlBuffers()
{
    m_heightBuffer->setData(*m_heightField, GL_DYNAMIC_DRAW);

    // TODO update needed data only
    /*for (; !m_bufferUpdateList.empty(); m_bufferUpdateList.pop_front()) {
        auto updateRange = m_bufferUpdateList.front();

    }*/
}
