#include "terraintile.h"

#include <cmath>

#include <glow/Texture.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/global.h>
#include "utils/cameraex.h"

#include <glm/glm.hpp>

#include "utils/pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxRigidStatic.h>
#include <PxShape.h>
#include <PxMaterial.h>
#include <PxScene.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>
#ifdef PX_WINDOWS
#include <gpu/PxParticleGpu.h>
#endif

#include "terrain.h"
#include "elements.h"
#include "world.h"
#include "physicswrapper.h"

TerrainTile::TerrainTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames)
: m_tileID(tileID)
, m_terrain(terrain)
, m_elementNames(elementNames)
, m_isInitialized(false)
, m_heightTex(nullptr)
, m_heightBuffer(nullptr)
, m_program(nullptr)
, m_pxShape(nullptr)
{
    terrain.registerTile(tileID, *this);

    // compute position depending on TileID, which sets the row/column positions of the tile
    float minX = terrain.settings.tileSizeX() * (tileID.x - 0.5f);
    float minZ = terrain.settings.tileSizeZ() * (tileID.z - 0.5f);
    m_transform = glm::mat4(
        terrain.settings.sampleInterval(), 0, 0, 0,
        0, 1, 0, 0,
        0, 0, terrain.settings.sampleInterval(), 0,
        minX, 0, minZ, 1);

    clearBufferUpdateRange();

    m_heightField.resize(terrain.settings.rows * terrain.settings.columns);
}

TerrainTile::~TerrainTile()
{
}

const std::string & TerrainTile::elementAt(unsigned int row, unsigned int column) const
{
    return m_elementNames.at(elementIndexAt(row, column));
}

void TerrainTile::prepareDraw()
{
    if (!m_isInitialized)
        initialize();
    if (!m_bufferUpdateList.empty())
        updateBuffers();
}

void TerrainTile::bind(const CameraEx & camera)
{
    prepareDraw();

    if (!m_program)
        initializeProgram();

    assert(m_program);
    assert(m_heightTex);

    m_heightTex->bindActive(GL_TEXTURE0);

    m_program->use();
    m_program->setUniform("cameraposition", camera.eye());
    glm::mat4 modelView = camera.view() * m_transform;
    m_program->setUniform("modelView", modelView);
    glm::mat4 modelViewProjection = camera.viewProjectionEx() * m_transform;
    m_program->setUniform("modelViewProjection", modelViewProjection);
    m_program->setUniform("znear", camera.zNearEx());
    m_program->setUniform("zfar", camera.zFarEx());
    m_terrain.setDrawGridOffsetUniform(*m_program, camera.eye());

    m_terrain.m_world.setUpLighting(*m_program);
}

void TerrainTile::unbind()
{
    m_program->release();

    m_heightTex->unbindActive(GL_TEXTURE0);
}

void TerrainTile::initialize()
{
    clearBufferUpdateRange();

    m_heightBuffer = new glow::Buffer(GL_TEXTURE_BUFFER);
    m_heightBuffer->setStorage(m_heightField, GL_MAP_WRITE_BIT);

    m_heightTex = new glow::Texture(GL_TEXTURE_BUFFER);
    m_heightTex->bind();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_heightBuffer->id());
    m_heightBuffer->unbind();

    m_isInitialized = true;
}

void TerrainTile::initializeProgram()
{
    m_program->setUniform("modelTransform", m_transform);
    m_program->setUniform("heightField", 0);
    m_program->setUniform("tileRowsColumns", glm::ivec2(m_terrain.settings.rows, m_terrain.settings.columns));

    Elements::setAllUniforms(*m_program);
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

    // create the list of material references
    PxHeightFieldSample * hfSamples = new PxHeightFieldSample[numSamples];
    PxMaterial ** materials = new PxMaterial*[m_elementNames.size()];
    for (uint8_t i = 0; i < m_elementNames.size(); ++i)
        materials[i] = Elements::pxMaterial(m_elementNames.at(i));

    // scale height so that we use the full range of PxI16=short
    PxReal heightScaleToWorld = m_terrain.settings.maxHeight / std::numeric_limits<PxI16>::max();
    assert(heightScaleToWorld >= PX_MIN_HEIGHTFIELD_Y_SCALE);
    float heightScaleToPx = std::numeric_limits<PxI16>::max() / m_terrain.settings.maxHeight;

    // copy the material and height data into the px heightfield
    for (unsigned int row = 0; row < m_terrain.settings.rows; ++row) {
        const unsigned int rowOffset = row * m_terrain.settings.columns;
        for (unsigned int column = 0; column < m_terrain.settings.columns; ++column) {
            const unsigned int index = column + rowOffset;
            hfSamples[index].materialIndex0 = hfSamples[index].materialIndex1 = elementIndexAt(row, column);
            hfSamples[index].height = static_cast<PxI16>(m_heightField.at(index) * heightScaleToPx);
        }
    }

    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbRows = m_terrain.settings.rows;
    hfDesc.nbColumns = m_terrain.settings.columns;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof(PxHeightFieldSample);

    PxHeightField * pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    assert(m_terrain.settings.sampleInterval() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    // create height field geometry and set scale
    PxHeightFieldGeometry pxHfGeometry(pxHeightField, PxMeshGeometryFlags(),
        heightScaleToWorld, m_terrain.settings.sampleInterval(), m_terrain.settings.sampleInterval());
    m_pxShape = pxActor.createShape(pxHfGeometry, materials, 1);

    assert(m_pxShape);

#ifdef PX_WINDOWS
    if (PhysicsWrapper::physxGpuAvailable())
        PxParticleGpu::createHeightFieldMirror(*pxHeightField, *PhysicsWrapper::getInstance()->cudaContextManager());
#endif

    delete[] hfSamples;
    delete[] materials;
}

float TerrainTile::heightAt(unsigned int row, unsigned int column) const
{
    return m_heightField.at(column + row * m_terrain.settings.columns);
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
    m_heightField.at(column + row * m_terrain.settings.columns) = value;
}

void TerrainTile::setElement(unsigned int row, unsigned int column, const std::string & elementName)
{
    uint8_t index = elementIndex(elementName);
    setElement(row, column, index);
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

glm::mat4 TerrainTile::transform() const
{
    return m_transform;
}

void TerrainTile::updateBuffers()
{
    assert(m_updateRangeMinMaxIndex.x < m_updateRangeMinMaxIndex.y);

    float * bufferDest = reinterpret_cast<float*>(m_heightBuffer->mapRange(
        sizeof(float) * m_updateRangeMinMaxIndex.x,
        sizeof(float) * (m_updateRangeMinMaxIndex.y - m_updateRangeMinMaxIndex.x),
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

    assert(bufferDest);

    unsigned int indexOffset = m_updateRangeMinMaxIndex.x;

    for (; !m_bufferUpdateList.empty(); m_bufferUpdateList.pop_front()) {
        UpdateRange range = m_bufferUpdateList.front();
        assert(indexOffset <= range.startIndex);
        assert(range.startIndex - indexOffset >= 0);
        assert(range.startIndex - indexOffset + range.nbElements < m_heightField.size());
        memcpy(bufferDest + (range.startIndex - indexOffset),
            reinterpret_cast<float*>(m_heightField.data()) + range.startIndex,
            range.nbElements * sizeof(float));
    }

    m_heightBuffer->unmap();

    updatePxHeight();
    clearBufferUpdateRange();
}

void TerrainTile::addBufferUpdateRange(unsigned int startIndex, unsigned int nbElements)
{
    if (m_updateRangeMinMaxIndex.x > startIndex)
        m_updateRangeMinMaxIndex.x = startIndex;
    if (m_updateRangeMinMaxIndex.y < startIndex + nbElements)
        m_updateRangeMinMaxIndex.y = startIndex + nbElements;
    m_bufferUpdateList.push_front({ startIndex, nbElements });
}

void TerrainTile::clearBufferUpdateRange()
{
    m_bufferUpdateList.clear();
    m_updateRangeMinMaxIndex = glm::detail::tvec2<unsigned int>(std::numeric_limits<unsigned int>::max(), 0);
    m_pxUpdateBox = UIntBoundingBox();
}

TerrainTile::UIntBoundingBox::UIntBoundingBox()
: minRow(std::numeric_limits<unsigned int>::max())
, maxRow(std::numeric_limits<unsigned int>::min())
, minColumn(std::numeric_limits<unsigned int>::max())
, maxColumn(std::numeric_limits<unsigned int>::min())
{
}

void TerrainTile::addToPxUpdateBox(unsigned int minRow, unsigned int maxRow, unsigned int minColumn, unsigned int maxColumn)
{
    if (m_pxUpdateBox.minRow > minRow)
        m_pxUpdateBox.minRow = minRow;
    if (m_pxUpdateBox.maxRow < maxRow)
        m_pxUpdateBox.maxRow = maxRow;
    if (m_pxUpdateBox.minColumn > minColumn)
        m_pxUpdateBox.minColumn = minColumn;
    if (m_pxUpdateBox.maxColumn < maxColumn)
        m_pxUpdateBox.maxColumn = maxColumn;
}

void TerrainTile::updatePxHeight()
{
    PxHeightFieldGeometry geometry;
    bool result = m_pxShape->getHeightFieldGeometry(geometry);
    assert(result);
    if (!result) {
        glow::warning("TerrainInteractor::setPxHeight could not get heightfield geometry from px shape");
        return;
    }
    PxHeightField * hf = geometry.heightField;

    assert(m_pxUpdateBox.minRow <= m_pxUpdateBox.maxRow && m_pxUpdateBox.minColumn <= m_pxUpdateBox.maxColumn);
    unsigned int nbRows = m_pxUpdateBox.maxRow - m_pxUpdateBox.minRow + 1;
    unsigned int nbColumns = m_pxUpdateBox.maxColumn - m_pxUpdateBox.minColumn + 1;
    unsigned int fieldSize = nbRows * nbColumns;

    PxHeightFieldSample * samplesM = new PxHeightFieldSample[fieldSize];
    for (unsigned int r = 0; r < nbRows; ++r) {
        unsigned int rowOffset = r * nbColumns;
        for (unsigned int c = 0; c < nbColumns; ++c) {
            const unsigned int index = c + rowOffset;
            const float terrainHeight = heightAt(r + m_pxUpdateBox.minRow, c + m_pxUpdateBox.minColumn);
            samplesM[index].height = static_cast<PxI16>(terrainHeight / geometry.heightScale);
            samplesM[index].materialIndex0 = samplesM[index].materialIndex1 = elementIndexAt(r + m_pxUpdateBox.minRow, c + m_pxUpdateBox.minColumn);
        }
    }

    PxHeightFieldDesc descM;
    descM.nbColumns = nbColumns;
    descM.nbRows = nbRows;
    descM.samples.data = samplesM;
    descM.format = hf->getFormat();
    descM.samples.stride = hf->getSampleStride();
    descM.thickness = hf->getThickness();
    descM.convexEdgeThreshold = hf->getConvexEdgeThreshold();
    descM.flags = hf->getFlags();

    PhysicsWrapper::getInstance()->pauseGPUAcceleration();

    bool success = hf->modifySamples(m_pxUpdateBox.minColumn, m_pxUpdateBox.minRow, descM);
    assert(success);
    if (!success) {
        glow::warning("TerrainInteractor::setPxHeight could not modify heightfield.");
        return;
    }

    PxHeightFieldGeometry newGeometry(hf, PxMeshGeometryFlags(), geometry.heightScale, geometry.rowScale, geometry.columnScale);
    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    pxScenePtrs[0]->lockWrite();
    m_pxShape->setGeometry(newGeometry);
    pxScenePtrs[0]->unlockWrite();

    PhysicsWrapper::getInstance()->restoreGPUAccelerated();

#ifdef PX_WINDOWS
    if (PhysicsWrapper::getInstance()->physxGpuAvailable()) {
        PxParticleGpu::releaseHeightFieldMirror(*hf);
        PxParticleGpu::createHeightFieldMirror(*hf, *PhysicsWrapper::getInstance()->cudaContextManager());
    }
#endif
}
