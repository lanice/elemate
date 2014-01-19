#include "drawable.h"

#include <glow/Program.h>
#include <glow/VertexArrayObject.h>
#include <glow/Buffer.h>

#include "cameraex.h"

Drawable::Drawable(const World & world)
: m_world(world)
, m_vao(nullptr)
, m_indexBuffer(nullptr)
, m_vbo(nullptr)
{
}

Drawable::~Drawable()
{
}

void Drawable::initialize()
{
}

void Drawable::draw(const glowutils::Camera & camera)
{
    if (!m_vao)
        initialize();

    assert(m_vao);
    assert(m_vbo);

    m_vao->bind();

    drawImplementation(camera);

    m_vao->unbind();
}

void Drawable::drawLightMap(const CameraEx & lightSource)
{
    if (!m_vao)
        initialize();
    if (!m_lightMapProgram)
        initLightMappingProgram();

    assert(m_vao);
    assert(m_indexBuffer);
    assert(m_vbo);
    assert(m_lightMapProgram);

    m_lightMapProgram->use();
    m_vao->bind();

    drawLightMapImpl(lightSource);

    m_vao->unbind();
    m_lightMapProgram->release();
}

void Drawable::drawShadowMapping(const glowutils::Camera & camera, const CameraEx & lightSource)
{
    if (!m_vao)
        initialize();
    if (!m_shadowMappingProgram)
        initShadowMappingProgram();

    assert(m_vao);
    assert(m_indexBuffer);
    assert(m_vbo);
    assert(m_shadowMappingProgram);

    m_shadowMappingProgram->use();

    m_shadowMappingProgram->setUniform("invViewportSize", 1.0f / ((camera.viewport().x + camera.viewport().y) * 0.5f));
    /*m_shadowMappingProgram->setUniform("znear", camera.zNear());
    m_shadowMappingProgram->setUniform("zfar", camera.zFar());*/

    m_vao->bind();

    drawShadowMappingImpl(camera, lightSource);

    m_vao->unbind();
    m_shadowMappingProgram->use();
}
