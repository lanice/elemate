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
#include "particlewaterstep.h"
#include "shadowmappingstep.h"

Renderer::Renderer(const World & world)
: m_world(world)
{
    initialize();
}

void Renderer::initialize()
{
    glow::DebugMessageOutput::enable();

    glClearColor(1, 1, 1, 1);

    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

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

    m_sceneFbo = new glow::FrameBufferObject();
    m_sceneFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_sceneColor);
    m_sceneFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_sceneDepth);
    m_sceneFbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_sceneFbo->unbind();

    m_particleWaterStep = std::make_shared<ParticleWaterStep>();
    m_shadowMappingStep = std::make_shared<ShadowMappingStep>(this->m_world);

    m_steps.push_back(m_particleWaterStep.get());
    m_steps.push_back(m_shadowMappingStep.get());

    m_handDepth = new glow::RenderBufferObject();
    // draw the hand into the scene color texture, but do not change the scene depth
    m_handFbo = new glow::FrameBufferObject();
    m_handFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_sceneColor);
    m_handFbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_handDepth);
    m_handFbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_handFbo->unbind();

    m_quadProgram = new glow::Program();
    m_quadProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/flush.frag"));

    m_quadProgram->setUniform("sceneColor", 0);
    m_quadProgram->setUniform("sceneDepth", 1);
    m_quadProgram->setUniform("waterNormals", 2);
    m_quadProgram->setUniform("waterDepth", 3);
    m_quadProgram->setUniform("shadowMap", 4);
    m_quadProgram->setUniform("lightMap", 5);

    m_quad = new glowutils::ScreenAlignedQuad(m_quadProgram);
}

void Renderer::addSceneFboReader(const std::function<void()> & reader)
{
    m_sceneFboReader.push_back(reader);
}

void Renderer::operator()(const glowutils::Camera & camera)
{
    assert(m_sceneFbo);

    sceneStep(camera);
    handStep(camera);
    m_particleWaterStep->draw(camera);
    m_shadowMappingStep->draw(camera);
    flushStep();
}

void Renderer::sceneStep(const glowutils::Camera & camera)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_sceneFbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_world.terrain->draw(camera);

    // call functions that read from the scene fbo for the object they are member of
    for (auto & function : m_sceneFboReader)
        function();

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

void Renderer::flushStep()
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    m_sceneColor->bind(GL_TEXTURE0);
    m_sceneDepth->bind(GL_TEXTURE1);
    m_particleWaterStep->normalsTex()->bind(GL_TEXTURE2);
    m_particleWaterStep->depthTex()->bind(GL_TEXTURE3);
    m_shadowMappingStep->result()->bind(GL_TEXTURE4);
    m_shadowMappingStep->lightMap()->bind(GL_TEXTURE5);

    m_quad->draw();

    m_sceneColor->unbind(GL_TEXTURE0);
    m_sceneDepth->unbind(GL_TEXTURE1);
    m_particleWaterStep->normalsTex()->unbind(GL_TEXTURE2);
    m_particleWaterStep->depthTex()->unbind(GL_TEXTURE3);
    m_shadowMappingStep->result()->unbind(GL_TEXTURE4);
    m_shadowMappingStep->lightMap()->unbind(GL_TEXTURE5);
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

    for (auto step : m_steps) {
        assert(step);
        step->resize(width, height);
    }
}
