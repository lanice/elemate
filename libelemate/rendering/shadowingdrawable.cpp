#include "shadowingdrawable.h"

#include <glow/Program.h>
#include <glow/VertexArrayObject.h>
#include <glow/Buffer.h>
#include "utils/cameraex.h"

ShadowingDrawable::ShadowingDrawable()
: Drawable()
, m_indexBuffer(nullptr)
{
}

void ShadowingDrawable::drawDepthMap(const CameraEx & camera)
{
    if (!m_vao)
        initialize();
    if (!m_depthMapProgram)
        initDepthMapProgram();

    assert(m_vao);
    assert(m_indexBuffer);
    assert(m_vbo);
    assert(m_depthMapProgram);

    m_vao->bind();

    drawDepthMapImpl(camera);

    m_vao->unbind();
}

void ShadowingDrawable::drawShadowMapping(const CameraEx & camera, const CameraEx & lightSource)
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
    m_shadowMappingProgram->setUniform("znear", camera.zNearEx());
    m_shadowMappingProgram->setUniform("zfar", camera.zFarEx());

    m_vao->bind();

    drawShadowMappingImpl(camera, lightSource);

    m_vao->unbind();
    m_shadowMappingProgram->use();
}
