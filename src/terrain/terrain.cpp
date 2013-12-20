#include "terrain.h"

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
, m_vbo(nullptr)
, m_heightTex(nullptr)
, m_program(nullptr)
, m_heightField(nullptr)
, m_vertices(nullptr)
, m_settings(settings)
, m_tileID(tileID)
{
}

Terrain::~Terrain()
{
    delete m_heightField;
    delete m_vertices;
}

void Terrain::draw(const glowutils::Camera & camera) 
{
    if (!m_vao)
        initialize();

    assert(m_program);
    assert(m_vao);


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

    glEnable(GL_PROGRAM_POINT_SIZE);
    m_vao->drawArrays(GL_POINTS, 0, m_vertices->size());

    m_vao->unbind();

    m_program->release();

    m_heightTex->unbind();
}

void Terrain::initialize()
{
    assert(m_heightField);
    assert(m_vertices);

    m_vao = new glow::VertexArrayObject();

    m_vao->bind();

    m_vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    m_vbo->setData(*m_vertices, GL_STATIC_DRAW);

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(m_vbo, 0, sizeof(glm::vec2));
    vertexBinding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

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

