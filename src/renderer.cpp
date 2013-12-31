#include "renderer.h"

#include <glow/global.h>
namespace glow {
    class Buffer; // missing forward declaration in FrameBufferObject.h
}
#include <glow/FrameBufferObject.h>
#include <glow/FrameBufferAttachment.h>
#include <glow/Texture.h>
#include <glow/RenderBufferObject.h>
#include <glow/Program.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>
#include <glowutils/ScreenAlignedQuad.h>

#include <cassert>

#include "world.h"
#include "terrain/terrain.h"
#include "particledrawable.h"
#include "hand.h"

Renderer::Renderer(const World & world)
: m_world(world)
{
    initialize();
}

void Renderer::initialize()
{
    glow::DebugMessageOutput::enable();

    glClearColor(1, 1, 1, 1);

    m_sceneColor = new glow::Texture(GL_TEXTURE_2D);
    m_sceneColor->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_sceneColor->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_sceneColor->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_sceneColor->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_sceneDepth = new glow::Texture(GL_TEXTURE_2D);
    m_sceneDepth->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_sceneDepth->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_sceneDepth->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_sceneDepth->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_sceneDepth->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
    m_sceneDepth->setParameter(GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

    m_sceneFbo = new glow::FrameBufferObject();
    m_sceneFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_sceneColor);
    m_sceneFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_sceneDepth);
    m_sceneFbo->setDrawBuffer({ GL_COLOR_ATTACHMENT0 });
    m_sceneFbo->unbind();


    m_handDepth = new glow::RenderBufferObject();

    // draw the hand into the scene color texture, but do not change the scene depth
    m_handFbo = new glow::FrameBufferObject();
    m_handFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_sceneColor);
    m_handFbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_handDepth);
    m_handFbo->setDrawBuffer({ GL_COLOR_ATTACHMENT0 });
    m_handFbo->unbind();


    m_particleWaterDepth = new glow::Texture(GL_TEXTURE_2D);
    m_particleWaterDepth->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_particleWaterDepth->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_particleWaterDepth->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_particleWaterDepth->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_particleWaterDepth->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
    m_particleWaterDepth->setParameter(GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

    m_particleWaterFbo = new glow::FrameBufferObject();
    m_particleWaterFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_particleWaterDepth);
    m_particleWaterFbo->setDrawBuffer(GL_NONE);
    m_particleWaterFbo->unbind();

    m_quadProgram = new glow::Program();
    m_quadProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/flush.frag"));

    m_quadProgram->setUniform("sceneColor", 0);
    m_quadProgram->setUniform("sceneDepth", 1);
    m_quadProgram->setUniform("waterDepth", 2);

    m_quad = new glowutils::ScreenAlignedQuad(m_quadProgram);
}

void Renderer::operator()(const glowutils::Camera & camera)
{
    assert(m_sceneFbo);

    sceneStep(camera);
    handStep(camera);
    particleWaterStep(camera);
    flushStep();
}

void Renderer::sceneStep(const glowutils::Camera & camera)
{
    m_sceneFbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    m_world.terrain->draw(camera);

    m_sceneFbo->unbind();
}

void Renderer::handStep(const glowutils::Camera & camera)
{
    m_handFbo->bind();

    // reuse the depth data from scene step
    // ... there are probably more performant ways for the hand depth test than copying the whole buffer...
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_sceneFbo->id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_handFbo->id());
    glBlitFramebuffer(0, 0, camera.viewport().x, camera.viewport().y, 0, 0, camera.viewport().x, camera.viewport().y,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    m_world.hand->draw(camera);
    
    m_handFbo->unbind();
}

void Renderer::particleWaterStep(const glowutils::Camera & camera)
{
    m_particleWaterFbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    ParticleDrawable::drawParticles(camera);

    m_particleWaterFbo->unbind();
}

void Renderer::flushStep()
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    m_sceneColor->bind(GL_TEXTURE0);
    m_sceneDepth->bind(GL_TEXTURE1);
    m_particleWaterDepth->bind(GL_TEXTURE2);

    m_quad->draw();

    m_sceneColor->unbind(GL_TEXTURE0);
    m_sceneDepth->unbind(GL_TEXTURE1);
    m_particleWaterDepth->unbind(GL_TEXTURE2);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

const glow::FrameBufferObject *  Renderer::sceneFbo() const
{
    return m_sceneFbo.get();
}

void Renderer::resize(int width, int height)
{
    m_sceneColor->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    m_sceneDepth->image2D(0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_sceneFbo->printStatus(true);
    assert(m_sceneFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_handDepth->storage(GL_DEPTH_COMPONENT32F, width, height);
    assert(m_handFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_particleWaterDepth->image2D(0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_particleWaterFbo->printStatus(true);
    assert(m_particleWaterFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);
}
