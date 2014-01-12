#include "terrain.h"

#include <glow/VertexArrayObject.h>
#include <glow/Texture.h>
#include <glow/Buffer.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "terraintile.h"


//http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
glm::vec3 lightInvDir(0.0, 2.0, 3.0);

//// Compute the MVP matrix from the light's point of view
float zNear = -10;
float zFar = 20;
glm::mat4 depthProjectionMatrix /*= glm::ortho<float>(-10, 10, -10, 10, zNear, zFar)*/;
glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
//glm::mat4 depthModelMatrix = glm::mat4(1.0);
//glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

void Terrain::drawLightMap(const glowutils::Camera & /*lightSource*/)
{
    // we probably don't want to draw an empty terrain
    assert(m_tiles.size() > 0);

    if (!m_vao)
        initialize();

    if (!m_lightMapProgram)
        initLightMapProgram();

    assert(m_vao);
    assert(m_indexBuffer);
    assert(m_vbo);

    m_vao->bind();

    glEnable(GL_PRIMITIVE_RESTART);

    m_lightMapProgram->use();

    m_lightMapProgram->setUniform("depthMVP", depthProjectionMatrix * depthViewMatrix * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_lightMapProgram->setUniform("viewport", glm::ivec2(1024, 1024));
    m_lightMapProgram->setUniform("znear", zNear);
    m_lightMapProgram->setUniform("zfar", zFar);

    // TODO: generalize for more tiles...

    m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_heightTex->bind(GL_TEXTURE0);
    m_tiles.at(TileID(TerrainLevel::WaterLevel))->m_heightTex->bind(GL_TEXTURE1);

    glPrimitiveRestartIndex(s_restartIndex);
    m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

    m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_heightTex->unbind(GL_TEXTURE0);
    m_tiles.at(TileID(TerrainLevel::WaterLevel))->m_heightTex->unbind(GL_TEXTURE1);

    m_lightMapProgram->release();

    m_vao->unbind();
}

glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
    );
//glm::mat4 depthBiasMVP = biasMatrix*depthMVP;

glow::Vec2Array depthSamples;

void Terrain::drawShadowMapping(const glowutils::Camera & camera, const glowutils::Camera & /*lightSource*/)
{
    if (!m_shadowMappingProgram)
        initShadowMappingProgram();

    m_vao->bind();

    glEnable(GL_PRIMITIVE_RESTART);

    // TODO: generalize for more tiles...

    auto baseTile = m_tiles.at(TileID(TerrainLevel::BaseLevel));
    auto waterTile = m_tiles.at(TileID(TerrainLevel::WaterLevel));

    m_shadowMappingProgram->use();

    glm::mat4 depthBiasMVP = biasMatrix * depthProjectionMatrix * depthViewMatrix * baseTile->transform();

    m_shadowMappingProgram->setUniform("modelTransform", baseTile->transform());
    m_shadowMappingProgram->setUniform("modelViewProjection", camera.viewProjection() * m_tiles.at(TileID(TerrainLevel::BaseLevel))->m_transform);
    m_shadowMappingProgram->setUniform("lightSourceView", depthViewMatrix);
    m_shadowMappingProgram->setUniform("invView", camera.viewInverted());
    m_shadowMappingProgram->setUniform("viewport", camera.viewport());
    m_shadowMappingProgram->setUniform("znear", camera.zNear());
    m_shadowMappingProgram->setUniform("zfar", camera.zFar());
    m_shadowMappingProgram->setUniform("depthBiasMVP", depthBiasMVP);

    baseTile->m_heightTex->bind(GL_TEXTURE1);
    waterTile->m_heightTex->bind(GL_TEXTURE2);

    glPrimitiveRestartIndex(s_restartIndex);
    m_vao->drawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(m_indices->size()), GL_UNSIGNED_INT, nullptr);

    baseTile->m_heightTex->unbind(GL_TEXTURE1);
    waterTile->m_heightTex->unbind(GL_TEXTURE2);

    m_lightMapProgram->release();

    m_vao->unbind();
}

void Terrain::initLightMapProgram()
{
    m_lightMapProgram = new glow::Program();

    m_lightMapProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/lightmap_terrain.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/lightmap_terrain.frag"));

    m_lightMapProgram->setUniform("heightField0", 0);
    m_lightMapProgram->setUniform("heightField1", 1);

    m_lightMapProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));

    float right = settings.sizeX * 0.5f;
    float top = settings.maxHeight;
    depthProjectionMatrix = glm::ortho<float>(-right, right, -top, top, -right, right);
}

void Terrain::initShadowMappingProgram()
{
    for (int i = 0; i < 32; ++i)
        depthSamples.push_back(glm::vec2(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f)));

    m_shadowMappingProgram = new glow::Program();

    m_shadowMappingProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/shadows/shadow_terrain.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/depth_util.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/shadows/shadow_terrain.frag"));

    m_shadowMappingProgram->setUniform("lightMap", 0);
    m_shadowMappingProgram->setUniform("heightField0", 1);
    m_shadowMappingProgram->setUniform("heightField1", 2);
    m_shadowMappingProgram->setUniform("depthSamples", depthSamples);

    m_shadowMappingProgram->setUniform("tileRowsColumns", glm::uvec2(settings.rows, settings.columns));
}
