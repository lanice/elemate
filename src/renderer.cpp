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

Renderer::Renderer(const World & world)
: m_world(world)
{
    initialize();
}

void Renderer::initialize()
{
    glClearColor(1, 1, 1, 1);

    m_colorFbo = new glow::FrameBufferObject();

    m_colorTex = new glow::Texture(GL_TEXTURE_2D);
    m_colorTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_colorTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_colorTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_colorTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_colorTex->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_depthBuffer = new glow::RenderBufferObject();

    m_colorFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_colorTex);
    m_colorFbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_depthBuffer);

    m_colorFbo->setDrawBuffer({ GL_COLOR_ATTACHMENT0 });

    m_colorFbo->unbind();

    m_quadProgram = new glow::Program();
    m_quadProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/flush.frag"));

    m_quadProgram->setUniform("colorBuffer", 0);
    m_quadProgram->setUniform("depthBuffer", 1);

    m_quad = new glowutils::ScreenAlignedQuad(m_quadProgram);
}

void Renderer::operator()(const glowutils::Camera & camera)
{
    glow::DebugMessageOutput::enable();

    assert(m_colorFbo);

    colorStep(camera);
    flushStep();
}

void Renderer::colorStep(const glowutils::Camera & camera)
{
    m_colorFbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    m_world.terrain->draw(camera);

    ParticleDrawable::drawParticles(camera);

    m_colorFbo->unbind();
}

void Renderer::flushStep()
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    m_colorTex->bind(GL_TEXTURE0);

    m_quad->draw();

    m_colorTex->unbind(GL_TEXTURE0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void Renderer::resize(int width, int height)
{
    m_colorTex->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    m_depthBuffer->storage(GL_DEPTH_COMPONENT32F, width, height);
}
