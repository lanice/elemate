#include "terrain.h"

#include <limits>
#include <algorithm>
#include <cmath>

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include "utils/cameraex.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "physicaltile.h"
#include "terraininteraction.h"

Terrain::Terrain(const TerrainSettings & settings)
: ShadowingDrawable()
, settings(settings)
, m_drawLevels(PhysicalLevels)
, m_viewRange(0.0f)
{
    TerrainInteraction::setDefaultTerrain(*this);
    m_boudingBox.extend(glm::vec3(settings.sizeX * 0.5f, settings.maxHeight, settings.sizeZ * 0.5f));
    m_boudingBox.extend(glm::vec3(-settings.sizeX * 0.5f, -settings.maxHeight, -settings.sizeZ * 0.5f));
}

void Terrain::draw(const CameraEx & camera, const std::initializer_list<std::string> & elements)
{
    setViewRange(camera.zFarEx());
    setDrawElements(elements);
    ShadowingDrawable::draw(camera);
    setDrawElements({});
}

void Terrain::drawDepthMap(const CameraEx & camera, const std::initializer_list<std::string> & elements)
{
    setDrawElements(elements);
    ShadowingDrawable::drawDepthMap(camera);
    setDrawElements({});
}

void Terrain::drawShadowMapping(const CameraEx & camera, const CameraEx & lightSource, const std::initializer_list<std::string> & elements)
{
    setDrawElements(elements);
    ShadowingDrawable::drawShadowMapping(camera, lightSource);
    setDrawElements({});
}

void Terrain::setDrawElements(const std::initializer_list<std::string> & elements)
{
    if (elements.size() == 0) {
        m_drawLevels = PhysicalLevels;
        return;
    }

    m_drawLevels.clear();
    for (const std::string & name : elements)
        m_drawLevels.insert(levelForElement.at(name));
}

const GLuint Terrain::s_restartIndex = std::numeric_limits<GLuint>::max();

void Terrain::drawImplementation(const CameraEx & camera)
{
    // we probably don't want to draw an empty terrain
    assert(m_physicalTiles.size() > 0);

    if (!m_renderGridRadius.isValid())
        generateDrawGrid();

    assert(m_indexBuffer);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    for (auto & pair : m_attributeTiles)
        pair.second->prepareDraw();

    for (auto & pair : m_physicalTiles) {
        if (m_drawLevels.find(pair.first.level) == m_drawLevels.end())
            continue;   // only draw elements that are listed for drawing
        pair.second->bind(camera);
        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
        pair.second->unbind();
    }

    glDisable(GL_CULL_FACE);

    glDisable(GL_PRIMITIVE_RESTART);
}

void Terrain::updatePhysics(float delta)
{
    for (auto & pair : m_attributeTiles)
        pair.second->updatePhysics(delta);
}

void Terrain::setDrawHeatMap(bool drawHeatMap)
{
    for (auto & pair : m_physicalTiles)
        pair.second->m_drawHeatMap = drawHeatMap;
}

const glowutils::AxisAlignedBoundingBox & Terrain::boudingBox() const
{
    return m_boudingBox;
}

void Terrain::setViewRange(float zfar)
{
    assert(zfar > 0);
    if (m_viewRange == zfar)
        return;
    
    m_viewRange = zfar;
    m_validBoudingBox.invalidate();
    m_renderGridRadius.invalidate();
}

const glowutils::AxisAlignedBoundingBox & Terrain::validBoundingBox() const
{
    if (m_validBoudingBox.isValid())
        return m_validBoudingBox.value();

    glowutils::AxisAlignedBoundingBox newBox;

    newBox.extend(glm::vec3(
        m_boudingBox.urb().x - m_viewRange,
        m_boudingBox.urb().y,
        m_boudingBox.urb().z - m_viewRange));
    newBox.extend(glm::vec3(
        m_boudingBox.llf().x + m_viewRange,
        m_boudingBox.llf().y,
        m_boudingBox.llf().z + m_viewRange));

    m_validBoudingBox.setValue(newBox);
    return m_validBoudingBox.value();
}

void Terrain::initialize()
{
    generateDrawGrid();

    m_vao = new glow::VertexArrayObject();

    m_indexBuffer = new glow::Buffer(GL_ELEMENT_ARRAY_BUFFER);
    m_indexBuffer->setData(m_indices, GL_STATIC_DRAW);

    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(m_vertices, GL_STATIC_DRAW);

    m_vao->bind();

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec2));
    vertexBinding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_indexBuffer->bind();

    m_vao->unbind();
}

void Terrain::setDrawGridOffsetUniform(glow::Program & program, const glm::vec3 & cameraposition) const
{
    assert(m_renderGridRadius.isValid());

    unsigned int offsetX = static_cast<unsigned int>(std::floor((0.5f + cameraposition.x/settings.sizeX) * settings.maxTileSamplesPerAxis - m_renderGridRadius.value()));
    unsigned int offsetZ = static_cast<unsigned int>(std::floor((0.5f + cameraposition.z / settings.sizeZ) * settings.maxTileSamplesPerAxis - m_renderGridRadius.value()));

    program.setUniform("rowColumnOffset", glm::ivec2(offsetX, offsetZ));
}

void Terrain::generateDrawGrid()
{
    m_renderGridRadius.setValue(static_cast<unsigned int>(std::ceil(m_viewRange * settings.maxSamplesPerWorldCoord())));
    unsigned int diameter = m_renderGridRadius.value() * 2;
    unsigned int numSamples = diameter * diameter;

    m_vertices.resize(numSamples);

    for (unsigned row = 0; row < diameter; ++row) {
        const unsigned rowOffset = row * diameter;
        for (unsigned column = 0; column < diameter; ++column) {
            unsigned int index = column + rowOffset;

            // simply use the row/column position as vertex position, scaling with the transform matrix
            m_vertices.at(index) = glm::vec2(row, column);
        }
    }


    // create a quad for all vertices, except for the last row and column (covered by the second last)
    // see PxHeightFieldDesc::samples documentation: "...(nbRows - 1) * (nbColumns - 1) cells are actually used."
    unsigned numIndices = (diameter - 1) * ((diameter) * 2 + 1);
    m_indices.reserve(numIndices);
    for (unsigned int row = 0; row < diameter - 1; ++row) {
        const unsigned rowOffset = row * diameter;
        for (unsigned int column = 0; column < diameter; ++column) {
            // "origin" is the left front vertex in a terrain quad
            const unsigned int origin = column + rowOffset;

            m_indices.push_back(origin);
            m_indices.push_back(origin + diameter);
        }
        m_indices.push_back(s_restartIndex);
    }

    assert(m_indices.size() == numIndices);
}

void Terrain::registerTile(const TileID & tileID, TerrainTile & tile)
{
    assert(m_physicalTiles.find(tileID) == m_physicalTiles.end());
    assert(m_attributeTiles.find(tileID) == m_attributeTiles.end());

    if (levelIsPhysical(tileID.level))
        m_physicalTiles.emplace(tileID, std::shared_ptr<TerrainTile>(&tile));
    else if (levelIsAttribute(tileID.level))
        m_attributeTiles.emplace(tileID, std::shared_ptr<TerrainTile>(&tile));
    else
        glow::fatal("Terrain: Trying to register a terrain tile with unknown level (%;)", int(tileID.level));
}

std::shared_ptr<TerrainTile> Terrain::getTile(TileID tileID) const
{
    if (levelIsPhysical(tileID.level))
        return m_physicalTiles.at(tileID);
    if (levelIsAttribute(tileID.level))
        return m_attributeTiles.at(tileID); 

    glow::fatal("Terrain: Trying to register a terrain tile with unknown level (%;)", int(tileID.level));
    assert(nullptr);
    return nullptr;
}

const std::map<TileID, physx::PxRigidStatic*> Terrain::pxActorMap() const
{
    return m_pxActors;
}

void Terrain::heighestLevelHeightAt(float x, float z, TerrainLevel & maxLevel, float & maxHeight) const
{
    maxHeight = std::numeric_limits<float>::lowest();
    maxLevel = TerrainLevel::BaseLevel;
    for (TerrainLevel level : PhysicalLevels) {
        float h = heightAt(x, z, level);
        if (h > maxHeight) {
            maxLevel = level;
            maxHeight = h;
        }
    }
}

TerrainLevel Terrain::heighestLevelAt(float x, float z) const
{
    TerrainLevel level; float height;
    heighestLevelHeightAt(x, z, level, height);
    return level;
}

float Terrain::heightTotalAt(float x, float z) const
{
    TerrainLevel level; float height;
    heighestLevelHeightAt(x, z, level, height);
    return height;
}

float Terrain::heightAt(float x, float z, TerrainLevel level) const
{
    float normX = 0.0f;
    float normZ = 0.0f;
    TileID tileID;
    if (!normalizePosition(x, z, tileID, normX, normZ))
        return 0.0f;

    tileID.level = level;

    return m_physicalTiles.at(tileID)->interpolatedValueAt(normX, normZ);
}

bool Terrain::worldToPhysicalTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<PhysicalTile> & physicalTile, unsigned int & row, unsigned int & column, float & row_fract, float & column_fract) const
{
    assert(std::find(PhysicalLevels.begin(), PhysicalLevels.end(), level) != PhysicalLevels.end());
    if (std::find(PhysicalLevels.begin(), PhysicalLevels.end(), level) == PhysicalLevels.end())
        return false;

    std::shared_ptr<TerrainTile> terrainTile;
    bool result = worldToTileRowColumn(x, z, level, terrainTile, row, column, row_fract, column_fract);

    if (!result)
        return false;

    physicalTile = std::dynamic_pointer_cast<PhysicalTile>(terrainTile);
    assert(physicalTile);

    return result;
}

bool Terrain::worldToPhysicalTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<PhysicalTile> & physicalTile, unsigned int & row, unsigned int & column) const
{
    float row_fract = 0.0f, column_fract = 0.0f;
    return worldToPhysicalTileRowColumn(x, z, level, physicalTile, row, column, row_fract, column_fract);
}

bool Terrain::worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column) const
{
    float row_fract = 0.0f, column_fract = 0.0f;
    return worldToTileRowColumn(x, z, level, terrainTile, row, column, row_fract, column_fract);
}

bool Terrain::worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column, float & row_fract, float & column_fract) const
{
    // only implemented for 1 tile
    assert(settings.tilesX == 1 && settings.tilesZ == 1);

    TileID tileID(level, 0, 0);
    terrainTile = getTile(tileID);
    assert(terrainTile);

    float normX = (x / settings.sizeX + 0.5f);
    float normZ = (z / settings.sizeZ + 0.5f);
    bool valid = normX >= 0 && normX <= 1 && normZ >= 0 && normZ <= 1;

    float row_int = 0.0f, column_int = 0.0f;
    row_fract = std::modf(normX * terrainTile->samplesPerAxis, &row_int);
    column_fract = std::modf(normZ * terrainTile->samplesPerAxis, &column_int);
    assert(row_int < terrainTile->samplesPerAxis && column_int < terrainTile->samplesPerAxis);

    row = static_cast<unsigned int>(row_int) % terrainTile->samplesPerAxis;
    column = static_cast<unsigned int>(column_int) % terrainTile->samplesPerAxis;

    return valid;
}

bool Terrain::normalizePosition(float x, float z, TileID & tileID, float & normX, float & normZ) const
{
    // currently for one tile only
    assert(settings.tilesX == 1 && settings.tilesZ == 1);
    tileID.x = 0;
    tileID.z = 0;

    normX = x / settings.sizeX + 0.5f;
    normZ = z / settings.sizeZ + 0.5f;

    return normX >= 0.0f && normX <= 1.0f
        && normZ >= 0.0f && normZ <= 1.0f;
}
