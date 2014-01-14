#include "drawable.h"

#include <glow/Program.h>
#include <glow/VertexArrayObject.h>
#include <glow/Buffer.h>

#include <glm/gtx/random.hpp>

#include "cameraex.h"

const glm::mat4 Drawable::s_biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0);

glow::Vec2Array initDepthSamples() {
    glow::Vec2Array samples;
    for (int i = 0; i < 32; ++i)
        samples.push_back(glm::vec2(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f)));
    return samples;
}
const glow::Vec2Array Drawable::s_depthSamples = initDepthSamples();

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
    m_vao->bind();

    drawShadowMappingImpl(camera, lightSource);

    m_vao->unbind();
    m_shadowMappingProgram->use();
}

void Drawable::initLightMappingProgram()
{
}

void Drawable::initShadowMappingProgram()
{
    m_shadowMappingProgram->setUniform("lightMap", 0);
    m_shadowMappingProgram->setUniform("depthSamples", s_depthSamples);
}
