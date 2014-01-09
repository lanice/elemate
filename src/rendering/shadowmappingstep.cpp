#include "shadowmappingstep.h"

namespace glow {
    class Buffer; // missing forward declaration in FrameBufferObject.h
}
#include <glow/FrameBufferObject.h>
#include <glow/Texture.h>
#include <glow/RenderBufferObject.h>

#include "world.h"
#include "terrain/terrain.h"

ShadowMappingStep::ShadowMappingStep(const World & world)
: m_world(world)
{
    m_lightTex = new glow::Texture(GL_TEXTURE_2D);
    m_lightTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_lightTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_lightTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_lightTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_depthBuffer = new glow::RenderBufferObject();

    m_fbo = new glow::FrameBufferObject();
    m_fbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_lightTex);
    m_fbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_depthBuffer);
    m_fbo->setDrawBuffer({ GL_COLOR_ATTACHMENT0 });
    m_fbo->unbind();
}

void ShadowMappingStep::draw(const glowutils::Camera & lightSource)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_fbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_world.terrain->drawShadow(lightSource);

    m_fbo->unbind();
}

void ShadowMappingStep::resize(int width, int height)
{
    m_lightTex->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    m_depthBuffer->storage(GL_DEPTH_COMPONENT32F, width, height);
    m_fbo->printStatus(true);
    assert(m_fbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);
}

glow::Texture * ShadowMappingStep::result()
{
    assert(m_lightTex);
    return m_lightTex.get();
}
