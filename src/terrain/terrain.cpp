#include "terrain.h"

#include <limits>

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>

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

