#include "shadowmappingstep.h"

#include <glow/Texture.h>
namespace glow {
    class Buffer; // missing forward declaration in FrameBufferObject.h
}
#include <glow/FrameBufferObject.h>

#include "world.h"
#include "terrain/terrain.h"

ShadowMappingStep::ShadowMappingStep(const World & world)
: m_world(world)
{
    m_tex = new glow::Texture(GL_TEXTURE_2D);
    m_tex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_tex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_tex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_tex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_tex->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);

    m_fbo = new glow::FrameBufferObject();
    m_fbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_tex);
    m_fbo->setDrawBuffer(GL_NONE);
    m_fbo->unbind();
}

void ShadowMappingStep::draw(const glowutils::Camera & lightSource)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_fbo->bind();

    glClear(GL_DEPTH_BUFFER_BIT);

    m_world.terrain->drawShadow(lightSource);

    m_fbo->unbind();
}

void ShadowMappingStep::resize(int width, int height)
{
    m_tex->image2D(0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fbo->printStatus(true);
    assert(m_fbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);
}

glow::Texture * ShadowMappingStep::result()
{
    assert(m_tex);
    return m_tex.get();
}
