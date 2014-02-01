#include "terrain.h"

#include <limits>
#include <algorithm>

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include "cameraex.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "terraintile.h"

Terrain::Terrain(const World & world, const TerrainSettings & settings)
: Drawable(world)
, settings(settings)
, m_vertices(nullptr)
, m_indices(nullptr)
, m_drawLevels(TerrainLevels)
, m_borderWidth(0.0f)
{
    m_boudingBox.extend(glm::vec3(settings.sizeX * 0.5f, settings.maxHeight, settings.sizeZ * 0.5f));
    m_boudingBox.extend(glm::vec3(-settings.sizeX * 0.5f, -settings.maxHeight, -settings.sizeZ * 0.5f));
}

Terrain::~Terrain()
{
    m_vertices->clear();
    delete m_vertices;
    m_indices->clear();
    delete m_indices;
}


void Terrain::draw(const CameraEx & camera, const std::initializer_list<std::string> & elements)
{
    setBorderWidth(camera.zFarEx());
    setDrawElements(elements);
    Drawable::draw(camera);
    setDrawElements({});
}

void Terrain::drawDepthMap(const CameraEx & camera, const std::initializer_list<std::string> & elements)
{
    setDrawElements(elements);
    Drawable::drawDepthMap(camera);
    setDrawElements({});
}

void Terrain::drawShadowMapping(const CameraEx & camera, const CameraEx & lightSource, const std::initializer_list<std::string> & elements)
{
    setDrawElements(elements);
    Drawable::drawShadowMapping(camera, lightSource);
    setDrawElements({});
}

void Terrain::setDrawElements(const std::initializer_list<std::string> & elements)
{
    if (elements.size() == 0) {
        m_drawLevels = TerrainLevels;
        return;
    }

    m_drawLevels.clear();
    for (const std::string & name : elements)
        m_drawLevels.insert(levelForElement(name));
}

const GLuint Terrain::s_restartIndex = std::numeric_limits<GLuint>::max();

void Terrain::drawImplementation(const CameraEx & camera)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    assert(m_indexBuffer);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    for (auto & pair : m_tiles) {
        if (m_drawLevels.find(pair.first.level) == m_drawLevels.end())
            continue;   // only draw elements that are listed for drawing
        pair.second->bind(camera);
        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);
        pair.second->unbind();
    }

    glDisable(GL_PRIMITIVE_RESTART);
}

const glowutils::AxisAlignedBoundingBox & Terrain::boudingBox() const
{
    return m_boudingBox;
}

void Terrain::setBorderWidth(float border)
{
    if (m_borderWidth == border)
        return;
    
    m_borderWidth = border;
    m_validBoudingBox.invalidate();
}

const glowutils::AxisAlignedBoundingBox & Terrain::validBoundingBox() const
{
    if (m_validBoudingBox.isValid())
        return m_validBoudingBox.value();

    glowutils::AxisAlignedBoundingBox newBox;

    newBox.extend(glm::vec3(
        m_boudingBox.urb().x - m_borderWidth,
        m_boudingBox.urb().y,
        m_boudingBox.urb().z - m_borderWidth));
    newBox.extend(glm::vec3(
        m_boudingBox.llf().x + m_borderWidth,
        m_boudingBox.llf().y,
        m_boudingBox.llf().z + m_borderWidth));

    m_validBoudingBox.setValue(newBox);
    return m_validBoudingBox.value();
}

void Terrain::initialize()
{
    generateVertices();
    generateIndices();

    m_vao = new glow::VertexArrayObject();

    m_indexBuffer = new glow::Buffer(GL_ELEMENT_ARRAY_BUFFER);
    m_indexBuffer->setData(*m_indices, GL_STATIC_DRAW);

    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(*m_vertices, GL_STATIC_DRAW);

    m_vao->bind();

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec2));
    vertexBinding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_indexBuffer->bind();

    m_vao->unbind();
}

void Terrain::generateVertices()
{
    unsigned int numSamples = settings.rows * settings.columns;

    m_vertices = new glow::Vec2Array();
    m_vertices->resize(numSamples);

    for (unsigned row = 0; row < settings.rows; ++row) {
        const unsigned rowOffset = row * settings.columns;
        for (unsigned column = 0; column < settings.columns; ++column) {
            unsigned int index = column + rowOffset;

            // simply use the row/column position as vertex position, scaling with the transform matrix
            m_vertices->at(index) = glm::vec2(row, column);
        }
    }
}

void Terrain::generateIndices()
{
    m_indices = new glow::UIntArray;

    // create a quad for all vertices, except for the last row and column (covered by the forelast)
    // see PxHeightFieldDesc::samples documentation: "...(nbRows - 1) * (nbColumns - 1) cells are actually used."
    unsigned numIndices = (settings.rows - 1) * ((settings.columns) * 2 + 1);
    m_indices->reserve(numIndices);
    for (unsigned int row = 0; row < settings.rows - 1; ++row) {
        const unsigned rowOffset = row * settings.columns;
        for (unsigned int column = 0; column < settings.columns; ++column) {
            // "origin" is the left front vertex in a terrain quad
            const unsigned int origin = column + rowOffset;

            m_indices->push_back(origin);
            m_indices->push_back(origin + settings.columns);
        }
        m_indices->push_back(s_restartIndex);
    }

    assert(m_indices->size() == numIndices);
}

void Terrain::registerTile(const TileID & tileID, TerrainTile & tile)
{
    assert(m_tiles.find(tileID) == m_tiles.end());

    m_tiles.emplace(tileID, std::shared_ptr<TerrainTile>(&tile));
}

const std::map<TileID, physx::PxRigidStatic*> Terrain::pxActorMap() const
{
    return m_pxActors;
}

void Terrain::heighestLevelHeightAt(float x, float z, TerrainLevel & maxLevel, float & maxHeight) const
{
    maxHeight = std::numeric_limits<float>::lowest();
    maxLevel = TerrainLevel::BaseLevel;
    for (TerrainLevel level : TerrainLevels) {
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
    std::shared_ptr<TerrainTile> tile = nullptr;
    float normX = 0.0f;
    float normZ = 0.0f;
    TileID tileID;
    if (!normalizePosition(x, z, tileID, normX, normZ))
        return 0.0f;

    tileID.level = level;

    return m_tiles.at(tileID)->interpolatedHeightAt(normX, normZ);
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
    float normX = (x / settings.sizeX + 0.5f);
    float normZ = (z / settings.sizeZ + 0.5f);
    bool valid = normX >= 0 && normX <= 1 && normZ >= 0 && normZ <= 1;

    float row_int = 0.0f, column_int = 0.0f;
    row_fract = std::modf(normX * settings.rows, &row_int);
    column_fract = std::modf(normZ * settings.columns, &column_int);

    row = static_cast<unsigned int>(row_int) % settings.rows;
    column = static_cast<unsigned int>(column_int) % settings.columns;

    TileID tileID(level, 0, 0);

    terrainTile = m_tiles.at(tileID);

    assert(terrainTile);

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
