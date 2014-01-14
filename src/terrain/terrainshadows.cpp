#include "terrain.h"

#include <glow/VertexArrayObject.h>
#include <glow/Texture.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>

#include <glm/gtc/matrix_transform.hpp>


#include "terraintile.h"
#include "cameraex.h"

void Terrain::drawLightMapImpl(const CameraEx & lightSource)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    m_lightMapProgram->setUniform("lightMVP", lightSource.viewProjectionOrthographic() * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_lightMapProgram->setUniform("viewport", glm::ivec2(1024, 1024));
    m_lightMapProgram->setUniform("znear", lightSource.zNearOrtho());
    m_lightMapProgram->setUniform("zfar", lightSource.zFar());

    // TODO: generalize for more tiles...

    m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_heightTex->bind(GL_TEXTURE0);
    m_tiles.at(TileID(TerrainLevel::WaterLevel))->m_heightTex->bind(GL_TEXTURE1);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

    glDisable(GL_PRIMITIVE_RESTART);

    m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_heightTex->unbind(GL_TEXTURE0);
    m_tiles.at(TileID(TerrainLevel::WaterLevel))->m_heightTex->unbind(GL_TEXTURE1);
}

void Terrain::drawShadowMappingImpl(const glowutils::Camera & camera, const CameraEx & lightSource)
{
    // TODO: generalize for more tiles...

    auto baseTile = m_tiles.at(TileID(TerrainLevel::BaseLevel));
    auto waterTile = m_tiles.at(TileID(TerrainLevel::WaterLevel));

    glm::mat4 lightBiasMVP = s_biasMatrix * lightSource.viewProjectionOrthographic() * baseTile->transform();

    m_shadowMappingProgram->setUniform("modelTransform", baseTile->transform());
    m_shadowMappingProgram->setUniform("modelViewProjection", camera.viewProjection() * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_shadowMappingProgram->setUniform("lightSourceView", lightSource.view());
    m_shadowMappingProgram->setUniform("invView", camera.viewInverted());
    m_shadowMappingProgram->setUniform("viewport", camera.viewport());
    m_shadowMappingProgram->setUniform("znear", camera.zNear());
    m_shadowMappingProgram->setUniform("zfar", camera.zFar());
    m_shadowMappingProgram->setUniform("lightBiasMVP", lightBiasMVP);

    baseTile->m_heightTex->bind(GL_TEXTURE1);
    waterTile->m_heightTex->bind(GL_TEXTURE2);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

    glDisable(GL_PRIMITIVE_RESTART);

    baseTile->m_heightTex->unbind(GL_TEXTURE1);
    waterTile->m_heightTex->unbind(GL_TEXTURE2);
}

void Terrain::initLightMappingProgram()
{
    m_lightMapProgram = new glow::Program();
    m_lightMapProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/lightmap_terrain.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/lightmap.frag"));

    m_lightMapProgram->setUniform("heightField0", 0);
    m_lightMapProgram->setUniform("heightField1", 1);

    m_lightMapProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));

    Drawable::initLightMappingProgram();
}

void Terrain::initShadowMappingProgram()
{
    m_shadowMappingProgram = new glow::Program();
    m_shadowMappingProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/shadowmapping_terrain.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/shadowmapping.frag"));

    m_shadowMappingProgram->setUniform("heightField0", 1);
    m_shadowMappingProgram->setUniform("heightField1", 2);

    m_shadowMappingProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));

    Drawable::initShadowMappingProgram();
}
