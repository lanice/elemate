#include "terrain.h"

#include <limits>
#include <algorithm>

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Program.h>

#include "terraintile.h"

Terrain::Terrain(const TerrainSettings & settings)
: settings(settings)
, m_vao(nullptr)
, m_indexBuffer(nullptr)
, m_vbo(nullptr)
, m_vertices(nullptr)
, m_indices(nullptr)
{
}

Terrain::~Terrain()
{
    delete m_vertices;
    delete m_indices;
}

const GLuint restartIndex = std::numeric_limits<GLuint>::max();

void Terrain::draw(const glowutils::Camera & camera)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    if (!m_vao)
        initialize();

    assert(m_vao);
    assert(m_indexBuffer);
    assert(m_vbo);

    m_vao->bind();

    glEnable(GL_PRIMITIVE_RESTART);

    for (auto & pair : m_tiles) {
        pair.second->bind(camera);
        glPrimitiveRestartIndex(restartIndex);
        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);
        //m_vao->drawArrays(GL_TRIANGLE_STRIP_ADJACENCY, 0, m_vertices->size());

        pair.second->unbind();
    }

    m_vao->unbind();
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
        m_indices->push_back(restartIndex);
    }

    assert(m_indices->size() == numIndices);
}

void Terrain::registerTile(const TileID & tileID, TerrainTile & tile)
{
    assert(m_tiles.find(tileID) == m_tiles.end());

    m_tiles.emplace(tileID, std::shared_ptr<TerrainTile>(&tile));
}

const std::unordered_map<TileID, physx::PxRigidStatic*> Terrain::pxActorMap() const
{
    return m_pxActors;
}

void Terrain::setUpLighting(glow::Program & program) const
{
    static glm::vec4 lightambientglobal(0, 0, 0, 0);
    static glm::vec3 lightdir1(0.0, 6.5, 7.5);
    static glm::vec3 lightdir2(0.0, -8.0, 7.5);

    static glm::mat4 lightMat1;
    lightMat1[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    lightMat1[1] = glm::vec4(0.2, 0.2, 0.2, 1.0);        //diffuse
    lightMat1[2] = glm::vec4(0.7, 0.7, 0.5, 1.0);        //specular
    lightMat1[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    static glm::mat4 lightMat2;
    lightMat2[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    lightMat2[1] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //diffuse
    lightMat2[2] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //specular
    lightMat2[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    program.setUniform("lightambientglobal", lightambientglobal);
    program.setUniform("lightdir1", lightdir1);
    program.setUniform("lightdir2", lightdir2);
    program.setUniform("light1", lightMat1);
    program.setUniform("light2", lightMat2);
}

float Terrain::heightAt(float x, float z) const
{
    float height = std::numeric_limits<float>::lowest();
    for (TerrainLevel level : TerrainLevels) {
        height = std::max(
            height,
            heightAt(x, z, level));
    }
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
    // only implemented for 1 tile
    assert(settings.tilesX == 1 && settings.tilesZ == 1);
    float normX = (x / settings.sizeX + 0.5f);
    float normZ = (z / settings.sizeZ + 0.5f);
    bool valid = normX >= 0 && normX <= 1 && normZ >= 0 && normZ <= 1;

    row = static_cast<int>(normX * settings.rows) % settings.rows;
    column = static_cast<int>(normZ * settings.columns) % settings.columns;

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
