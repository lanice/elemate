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
#include <glowutils/ScreenAlignedQuad.h>
#include "cameraex.h"

#include <cassert>

#include "world.h"
#include "terrain/terrain.h"
#include "particledrawable.h"
#include "hand.h"
#include "particlewaterstep.h"
#include "shadowmappingstep.h"
#include "ui/userinterface.h"

Renderer::Renderer(const World & world)
: m_world(world)
{
    initialize();
}

void Renderer::initialize()
{
    glow::DebugMessageOutput::enable();

    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

    NamedTexture sceneColor = { "sceneColor", new glow::Texture(GL_TEXTURE_2D) };
    sceneColor.second->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    sceneColor.second->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    sceneColor.second->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    sceneColor.second->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_textureByName.insert(sceneColor);

    NamedTexture sceneDepth = { "sceneDepth", new glow::Texture(GL_TEXTURE_2D) };
    sceneDepth.second->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    sceneDepth.second->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    sceneDepth.second->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    sceneDepth.second->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    sceneDepth.second->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
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
    m_textureByName.insert(handColor);

    NamedTexture handDepth = { "handDepth", new glow::Texture(GL_TEXTURE_2D) };
    handDepth.second->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    handDepth.second->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    handDepth.second->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    handDepth.second->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    handDepth.second->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
    m_textureByName.insert(handDepth);

    NamedFbo handFbo = { "hand", new glow::FrameBufferObject() };
    handFbo.second->attachTexture2D(GL_COLOR_ATTACHMENT0, handColor.second);
    handFbo.second->attachTexture2D(GL_DEPTH_ATTACHMENT, handDepth.second);
    handFbo.second->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    handFbo.second->unbind();
    m_fboByName.insert(handFbo);



    m_particleWaterStep = std::make_shared<ParticleWaterStep>();
    m_shadowMappingStep = std::make_shared<ShadowMappingStep>(this->m_world);

    m_steps.push_back(m_particleWaterStep.get());
    m_steps.push_back(m_shadowMappingStep.get());

    m_quadProgram = new glow::Program();
    m_quadProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/depth_util.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/material.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/flush.frag")
    );

    m_flushSources =
    { sceneColor, sceneDepth, handColor, handDepth,
    NamedTexture("waterNormals", m_particleWaterStep->normalsTex()),
    NamedTexture("waterDepth", m_particleWaterStep->depthTex()),
    NamedTexture("shadowMap", m_shadowMappingStep->result()),
    NamedTexture("lightMap", m_shadowMappingStep->lightMap())};

    for (int i = 0; i < m_flushSources.size(); ++i)
        m_quadProgram->setUniform(m_flushSources.at(i).first, i);

    m_quad = new glowutils::ScreenAlignedQuad(m_quadProgram);
}

void Renderer::addSceneFboReader(const std::function<void()> & reader)
{
    m_sceneFboReader.push_back(reader);
}

void Renderer::operator()(const CameraEx & camera)
{
    sceneStep(camera);
    handStep(camera);
    m_particleWaterStep->draw(camera);
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

    // call functions that read from the scene fbo for the object they are member of
    for (auto & function : m_sceneFboReader)
        function();

    m_fboByName.at("scene")->unbind();
}

void Renderer::handStep(const CameraEx & camera)
{
    m_fboByName.at("hand")->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_world.hand->draw(camera);
    
    m_fboByName.at("hand")->unbind();
}

void Renderer::flushStep(const CameraEx & camera)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    for (int i = 0; i < m_flushSources.size(); ++i)
        m_flushSources.at(i).second->bind(GL_TEXTURE0 + i);

    m_quad->program()->setUniform("znear", camera.zNearEx());
    m_quad->program()->setUniform("zfar", camera.zFarEx());
    m_quad->program()->setUniform("timef", int(m_world.getTime()));
    m_quad->program()->setUniform("view", camera.view());
    m_quad->program()->setUniform("camDirection", glm::normalize(camera.center() - camera.eye()));

    m_quad->draw();

    for (int i = 0; i < m_flushSources.size(); ++i)
        m_flushSources.at(i).second->unbind(GL_TEXTURE0 + i);
}

const glow::FrameBufferObject *  Renderer::sceneFbo() const
{
    return m_fboByName.at("scene");
}

void Renderer::resize(int width, int height)
{
    m_textureByName.at("sceneColor")->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    m_textureByName.at("sceneDepth")->image2D(0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fboByName.at("scene")->printStatus(true);
    // assert(m_fboByName.at("scene")->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_textureByName.at("handColor")->image2D(0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    m_textureByName.at("handDepth")->image2D(0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fboByName.at("hand")->printStatus(true);
    // assert(m_fboByName.at("hand")->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    for (auto step : m_steps) {
        assert(step);
        step->resize(width, height);
    }
}
