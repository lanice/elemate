#include "terrain.h"

#include <glow/VertexArrayObject.h>
#include <glow/Texture.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include "cameraex.h"

#include <glm/gtc/matrix_transform.hpp>


#include "terraintile.h"
#include "rendering/shadowmappingstep.h"
#include "world.h"

void Terrain::drawDepthMapImpl(const CameraEx & camera)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    m_depthMapProgram->setUniform("depthMVP", camera.viewProjectionEx() * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_depthMapProgram->setUniform("znear", camera.zNearEx());
    m_depthMapProgram->setUniform("zfar", camera.zFar());

    // TODO: generalize for more tiles...


    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    for (TerrainLevel level : m_drawLevels) {
        TerrainTile & tile = *m_tiles.at(TileID(level));

        tile.prepareDraw();
        tile.m_heightTex->bind(GL_TEXTURE0);

        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

        tile.m_heightTex->unbind(GL_TEXTURE0);
    }

    glDisable(GL_PRIMITIVE_RESTART);
}

void Terrain::drawShadowMappingImpl(const CameraEx & camera, const CameraEx & lightSource)
{
    // TODO: generalize for more tiles...

    auto baseTile = m_tiles.at(TileID(TerrainLevel::BaseLevel));

    glm::mat4 lightBiasMVP = ShadowMappingStep::s_biasMatrix * lightSource.viewProjectionEx() * baseTile->transform();

    m_shadowMappingProgram->setUniform("modelTransform", baseTile->transform());
    m_shadowMappingProgram->setUniform("modelViewProjection", camera.viewProjectionEx() * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_shadowMappingProgram->setUniform("lightBiasMVP", lightBiasMVP);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    for (TerrainLevel level : m_drawLevels) {
        TerrainTile & tile = *m_tiles.at(TileID(level));

        tile.prepareDraw();
        tile.m_heightTex->bind(GL_TEXTURE1);

        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

        tile.m_heightTex->unbind(GL_TEXTURE1);
    }

    glDisable(GL_PRIMITIVE_RESTART);
}

void Terrain::initDepthMapProgram()
{
    m_depthMapProgram = new glow::Program();
    m_depthMapProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/depthmap_terrain.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/depth_util.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/shadows/depthmap.frag"));

    m_depthMapProgram->setUniform("heightField", 0);

    m_depthMapProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));
}

void Terrain::initShadowMappingProgram()
{
    m_shadowMappingProgram = new glow::Program();
    m_shadowMappingProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/shadowmapping_terrain.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/shadows/shadowmapping.frag"));

    m_shadowMappingProgram->setUniform("heightField", 1);

    m_shadowMappingProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));

    ShadowMappingStep::setUniforms(*m_shadowMappingProgram);
}
