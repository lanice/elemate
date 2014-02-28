#include "terrain.h"

#include <glow/VertexArrayObject.h>
#include <glow/Texture.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/global.h>
#include "utils/cameraex.h"

#include <glm/gtc/matrix_transform.hpp>


#include "terraintile.h"
#include "rendering/shadowmappingstep.h"
#include "world.h"
#include "texturemanager.h"

void Terrain::drawDepthMapImpl(const CameraEx & camera)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    if (!m_renderGridRadius.isValid())
        generateDrawGrid();

    glow::Program * program = nullptr;

    // linearize the depth values for perspective projections
    if (camera.projectionType() == ProjectionType::perspective) {
        program = m_depthMapLinearizedProgram;
        program->setUniform("znear", camera.zNearEx());
        program->setUniform("zfar", camera.zFarEx());
    }
    else { // not needed for the orthographic projection
        program = m_depthMapProgram; 
    }

    setDrawGridOffsetUniform(*program, camera.eye());

    TerrainTile & baseTile = *m_tiles.at(TileID(TerrainLevel::BaseLevel));

    program->setUniform("depthMVP", camera.viewProjectionEx() * baseTile.m_transform);

    // TODO: generalize for more tiles...

    program->use();

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    for (TerrainLevel level : m_drawLevels) {
        TerrainTile & tile = *m_tiles.at(TileID(level));

        program->setUniform("baseTileCompare", level == TerrainLevel::WaterLevel);

        tile.prepareDraw();
        program->setUniform("heightField", TextureManager::getTextureUnit(tile.tileName, "heightField"));
        program->setUniform("baseHeightField", TextureManager::getTextureUnit(baseTile.tileName, "heightField"));

        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    glDisable(GL_CULL_FACE);

    glDisable(GL_PRIMITIVE_RESTART);

    program->release();
}

void Terrain::drawShadowMappingImpl(const CameraEx & camera, const CameraEx & lightSource)
{
    // TODO: generalize for more tiles...

    if (!m_renderGridRadius.isValid())
        generateDrawGrid();

    auto baseTile = m_tiles.at(TileID(TerrainLevel::BaseLevel));

    glm::mat4 lightBiasMVP = ShadowMappingStep::s_biasMatrix * lightSource.viewProjectionEx() * baseTile->transform();

    m_shadowMappingProgram->setUniform("modelTransform", baseTile->transform());
    m_shadowMappingProgram->setUniform("modelViewProjection", camera.viewProjectionEx() * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_shadowMappingProgram->setUniform("lightBiasMVP", lightBiasMVP);

    setDrawGridOffsetUniform(*m_shadowMappingProgram, camera.eye());

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(s_restartIndex);

    for (TerrainLevel level : m_drawLevels) {
        TerrainTile & tile = *m_tiles.at(TileID(level));

        tile.prepareDraw();
        m_shadowMappingProgram->setUniform("heightField", TextureManager::getTextureUnit(tile.tileName, "heightField"));

        m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    glDisable(GL_PRIMITIVE_RESTART);
}

void Terrain::initDepthMapProgram()
{
    m_depthMapProgram = new glow::Program();
    m_depthMapProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/shadows/depthmap_terrain.vert"),
        World::instance()->sharedShader(GL_GEOMETRY_SHADER, "shader/shadows/depthmap_terrain.geo"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/utils/passthrough.frag"));
    m_depthMapProgram->setUniform("tileRowsColumns", glm::ivec2(settings.rows, settings.columns));


    m_depthMapLinearizedProgram = new glow::Program();
    m_depthMapLinearizedProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/shadows/depthmap_terrain.vert"),
        World::instance()->sharedShader(GL_GEOMETRY_SHADER, "shader/shadows/depthmap_terrain.geo"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/utils/depth_util.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/shadows/depthmapLinearized.frag"));
    m_depthMapLinearizedProgram->setUniform("tileRowsColumns", glm::ivec2(settings.rows, settings.columns));
}

void Terrain::initShadowMappingProgram()
{
    m_shadowMappingProgram = new glow::Program();
    m_shadowMappingProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/shadowmapping_terrain.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/shadows/shadowmapping.frag"));

    m_shadowMappingProgram->setUniform("tileRowsColumns", glm::ivec2(settings.rows, settings.columns));

    ShadowMappingStep::setUniforms(*m_shadowMappingProgram);
}
