#include "terrain.h"

#include <limits>

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Texture.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>


#include <glm/glm.hpp>


TerrainSettings::TerrainSettings()
: sizeX(200)
, sizeZ(200)
, maxHeight(20.f)
, maxBasicHeightVariance(0.05f)
, rows(20)
, columns(20)
, tilesX(1)
, tilesZ(1)
{
}

Terrain::Terrain(const TileID & tileID, const TerrainSettings & settings)
: m_vao(nullptr)
, m_indexBuffer(nullptr)
, m_vbo(nullptr)
, m_heightTex(nullptr)
, m_program(nullptr)
, m_heightField(nullptr)
, m_vertices(nullptr)
, m_indices(nullptr)
, m_settings(settings)
, m_tileID(tileID)
{
}

Terrain::~Terrain()
{
    delete m_heightField;
    delete m_vertices;
    delete m_indices;
}

void Terrain::draw(const glowutils::Camera & camera) 
{
    if (!m_vao)
        initialize();

    assert(m_program);
    assert(m_vao);
    assert(m_indices);


    glActiveTexture(GL_TEXTURE0 + 0);
    m_heightTex->bind();

    m_program->use();
    const auto & viewProjection = camera.viewProjection();
    m_program->setUniform("viewProjection", viewProjection);
    glm::mat4 modelViewProjection = viewProjection * m_transform;
    m_program->setUniform("modelViewProjection", modelViewProjection);

    m_program->setUniform("heightField", 0);
    m_program->setUniform("texScale", glm::vec2(
        1.0f / m_settings.rows,
        1.0f / m_settings.columns));

    m_vao->bind();

    //glEnable(GL_PROGRAM_POINT_SIZE);
    glPrimitiveRestartIndex(std::numeric_limits<GLuint>::max());
    m_vao->drawElements(GL_TRIANGLE_STRIP, m_indices->size(), GL_UNSIGNED_INT, nullptr);
    //m_vao->drawArrays(GL_TRIANGLE_STRIP_ADJACENCY, 0, m_vertices->size());

    m_vao->unbind();

    m_program->release();

    m_heightTex->unbind();
}

void Terrain::initialize()
{
    assert(m_heightField);
    assert(m_vertices);

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

    m_heightTex = new glow::Texture(GL_TEXTURE_2D);
    m_heightTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_heightTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_heightTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_heightTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_heightTex->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    m_heightTex->image2D(0, GL_RED, m_settings.rows, m_settings.columns, 0,
        GL_RED, GL_FLOAT, m_heightField->rawData());

    glow::ref_ptr<glow::Shader> terrainBaseVert = glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/terrainBase.vert");
    glow::ref_ptr<glow::Shader> terrainBaseFrag = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/terrainBase.frag");
    glow::ref_ptr<glow::Shader> phongLightingFrag = glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/phongLighting.frag");

    m_program = new glow::Program();
    m_program->attach(terrainBaseFrag, terrainBaseVert, phongLightingFrag);
}

void Terrain::generateIndices()
{
    m_indices = new glow::UIntArray;

    // create a quad for all vertices, except for the last row and column (covered by the forelast)
    // we use 4 indices + primitive restart
    unsigned numIndices = (m_settings.rows - 1) * (m_settings.columns - 1) * 5;
    m_indices->reserve(numIndices);
    for (unsigned int row = 0; row < m_settings.rows - 1; ++row) {
        const unsigned rowOffset = row * m_settings.columns;
        for (unsigned int column = 0; column < m_settings.columns - 1; ++column) {
            // "origin" is the left front vertex in a terrain quad
            const unsigned int origin = column + rowOffset;

            // push back triangle strip for the quad + restart index
            m_indices->push_back(origin);
            m_indices->push_back(origin + 1);
            m_indices->push_back(origin + m_settings.columns);
            m_indices->push_back(origin + m_settings.columns + 1);
            m_indices->push_back(std::numeric_limits<GLuint>::max());
        }
    }

    assert(m_indices->size() == numIndices);
}

physx::PxRigidStatic * Terrain::pxActor() const
{
    assert(m_pxActor);
    return m_pxActor;
}

