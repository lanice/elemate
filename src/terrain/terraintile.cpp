#include "terraintile.h"

#include <glow/Texture.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include <glm/glm.hpp>

#include "terrain.h"

TerrainTile::TerrainTile(const TileID & tileID)
: m_tileID(tileID)
, m_terrain(nullptr)
, m_heightTex(nullptr)
, m_program(nullptr)
, m_heightField(nullptr)
{
}

TerrainTile::~TerrainTile()
{
    delete m_heightField;
}

void TerrainTile::bind(const glowutils::Camera & camera)
{
    if (!m_heightTex)
        initialize();
    if (!m_program)
        initializeProgram();

    assert(m_terrain);
    assert(m_program);
    assert(m_heightField);
    assert(m_heightTex);

    glActiveTexture(GL_TEXTURE0 + 0);
    m_heightTex->bind();

    m_program->use();
    const auto & viewProjection = camera.viewProjection();
    m_program->setUniform("viewProjection", viewProjection);
    glm::mat4 modelViewProjection = viewProjection * m_transform;
    m_program->setUniform("modelViewProjection", modelViewProjection);

    m_program->setUniform("heightField", 0);
    m_program->setUniform("texScale", glm::vec2(
        1.0f / m_terrain->settings.rows,
        1.0f / m_terrain->settings.columns));

}

void TerrainTile::unbind()
{
    m_program->release();

    m_heightTex->unbind();
}

void TerrainTile::initialize()
{
    assert(m_heightField);

    m_heightTex = new glow::Texture(GL_TEXTURE_2D);
    m_heightTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_heightTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_heightTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_heightTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_heightTex->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_heightTex->image2D(0, GL_R32F, m_terrain->settings.rows, m_terrain->settings.columns, 0,
        GL_RED, GL_FLOAT, m_heightField->rawData());
}
