#include "renderer.h"

namespace glow {
    class Buffer; // missing forward declaration in FrameBufferObject.h
}
#include <glow/FrameBufferObject.h>
#include <glow/FrameBufferAttachment.h>
#include <glow/Texture.h>
#include <glow/RenderBufferObject.h>
#include <glow/Program.h>
#include <glowutils/global.h>
#include <glowutils/ScreenAlignedQuad.h>
#include "utils/cameraex.h"

#include <cassert>

#include "world.h"
#include "terrain/terrain.h"
#include "ui/hand.h"
#include "ui/userinterface.h"
#include "particledrawable.h"
#include "particlestep.h"
#include "shadowmappingstep.h"
#include "debugstep.h"
#include "texturemanager.h"

Renderer::Renderer(const World & world)
: m_drawDebugStep(false)
, m_world(world)
{
    initialize();
}

void Renderer::initialize()
{
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

    NamedTexture sceneColor = { "sceneColor", new glow::Texture(GL_TEXTURE_2D) };
    sceneColor.second->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    sceneColor.second->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    sceneColor.second->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    sceneColor.second->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    sceneColor.second->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("Renderer", "sceneColor"));
    glActiveTexture(GL_TEXTURE0);
    m_textureByName.insert(sceneColor);

    NamedTexture sceneDepth = { "sceneDepth", new glow::Texture(GL_TEXTURE_2D) };
    sceneDepth.second->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    sceneDepth.second->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    sceneDepth.second->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    sceneDepth.second->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    sceneDepth.second->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
    sceneDepth.second->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("Renderer", "sceneDepth"));
    glActiveTexture(GL_TEXTURE0);
    m_textureByName.insert(sceneDepth);

    NamedFbo sceneFbo = { "scene", new glow::FrameBufferObject() };
    sceneFbo.second->attachTexture2D(GL_COLOR_ATTACHMENT0, sceneColor.second);
    sceneFbo.second->attachTexture2D(GL_DEPTH_ATTACHMENT, sceneDepth.second);
    sceneFbo.second->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    sceneFbo.second->unbind();
    m_fboByName.insert(sceneFbo);



    NamedTexture handColor = { "handColor", new glow::Texture(GL_TEXTURE_2D) };
    handColor.second->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    handColor.second->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    handColor.second->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    handColor.second->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    handColor.second->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("Renderer", "handColor"));
    glActiveTexture(GL_TEXTURE0);
    m_textureByName.insert(handColor);

    NamedTexture handDepth = { "handDepth", new glow::Texture(GL_TEXTURE_2D) };
    handDepth.second->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    handDepth.second->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    handDepth.second->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    handDepth.second->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    handDepth.second->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
    handDepth.second->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("Renderer", "handDepth"));
    glActiveTexture(GL_TEXTURE0);
    m_textureByName.insert(handDepth);

    NamedFbo handFbo = { "hand", new glow::FrameBufferObject() };
    handFbo.second->attachTexture2D(GL_COLOR_ATTACHMENT0, handColor.second);
    handFbo.second->attachTexture2D(GL_DEPTH_ATTACHMENT, handDepth.second);
    handFbo.second->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    handFbo.second->unbind();
    m_fboByName.insert(handFbo);



    m_particleStep = std::make_shared<ParticleStep>();
    m_shadowMappingStep = std::make_shared<ShadowMappingStep>(this->m_world);

    m_steps.push_back(m_particleStep.get());
    m_steps.push_back(m_shadowMappingStep.get());

    m_quadProgram = new glow::Program();
    m_quadProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/utils/depth_util.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/particles/material.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/flush.frag")
    );

    std::function<void(const std::string &)> setOwnSamplerUniform = [&](const std::string & name) {
        m_quadProgram->setUniform(name, TextureManager::getTextureUnit("Renderer", name));
    };
    std::function<void(const std::string &, const std::string &)> setSamplerUniform = [&](const std::string & owner, const std::string & name) {
        m_quadProgram->setUniform(name, TextureManager::getTextureUnit(owner, name));
    };
    setOwnSamplerUniform("sceneColor");
    setOwnSamplerUniform("sceneDepth");
    setOwnSamplerUniform("handColor");
    setOwnSamplerUniform("handDepth");
    setSamplerUniform("ParticleStep", "particleNormals");
    setSamplerUniform("ParticleStep", "particleDepth");
    setSamplerUniform("ParticleStep", "elementID");
    setSamplerUniform("ShadowMapping", "lightMap");
    setSamplerUniform("ShadowMapping", "shadowMap");

    m_quad = new glowutils::ScreenAlignedQuad(m_quadProgram);
    
    m_debugStep = std::make_shared<DebugStep>();
}

void Renderer::operator()(const CameraEx & camera)
{
    sceneStep(camera);
    handStep(camera);
    m_particleStep->draw(camera);
    m_shadowMappingStep->draw(camera);
    flushStep(camera);
}

void Renderer::sceneStep(const CameraEx & camera)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_fboByName.at("scene")->bind();

    glClearColor(m_world.skyColor().x, m_world.skyColor().y, m_world.skyColor().z, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_world.terrain->draw(camera, { "bedrock" });

    m_fboByName.at("scene")->unbind();
}

void Renderer::handStep(const CameraEx & camera)
{
    m_fboByName.at("hand")->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_world.hand->draw(camera);

    if (m_drawDebugStep)
        m_debugStep->draw(camera);  // rendered with the hand, to have it as a part of the scene
    
    m_fboByName.at("hand")->unbind();
}

void Renderer::flushStep(const CameraEx & camera)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    m_quad->program()->setUniform("znear", camera.zNearEx());
    m_quad->program()->setUniform("zfar", camera.zFarEx());
    m_quad->program()->setUniform("timef", int(m_world.getTime()));
    m_quad->program()->setUniform("view", camera.view());
    m_quad->program()->setUniform("camDirection", glm::normalize(camera.center() - camera.eye()));
    m_quad->program()->setUniform("viewport", camera.viewport());

    m_quad->draw();
}

const glow::FrameBufferObject *  Renderer::sceneFbo() const
{
    return m_fboByName.at("scene");
}

bool Renderer::drawDebugInfo() const
{
    return m_drawDebugStep;
}

void Renderer::toggleDrawDebugInfo()
{
    m_drawDebugStep = !m_drawDebugStep;
}

void Renderer::setDrawDebugInfo(bool doDraw)
{
    m_drawDebugStep = doDraw;
}

void Renderer::resize(int width, int height)
{
    m_textureByName.at("sceneColor")->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_textureByName.at("sceneDepth")->image2D(0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fboByName.at("scene")->printStatus(true);
    // assert(m_fboByName.at("scene")->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_textureByName.at("handColor")->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_textureByName.at("handDepth")->image2D(0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fboByName.at("hand")->printStatus(true);
    // assert(m_fboByName.at("hand")->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    for (auto step : m_steps) {
        assert(step);
        step->resize(width, height);
    }
}
