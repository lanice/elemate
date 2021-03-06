#include "renderer.h"

#include <fstream>
#include <ctime>

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
#include "io/imagereader.h"

Renderer::Renderer()
: m_drawDebugStep(false)
, m_drawHeatMap(false)
, m_takeScreenShot(false)
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
    m_shadowMappingStep = std::make_shared<ShadowMappingStep>();

    m_steps.push_back(m_particleStep.get());
    m_steps.push_back(m_shadowMappingStep.get());

    m_quadProgram = new glow::Program();
    m_quadProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/utils/depth_util.frag"),
        World::instance()->sharedShader(GL_FRAGMENT_SHADER, "shader/particles/material.frag"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/flush.frag")
    );

    initSkybox();

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
    setOwnSamplerUniform("skybox");

    World::instance()->setUpLighting(*m_quadProgram);

    m_quad = new glowutils::ScreenAlignedQuad(m_quadProgram);
    
    m_debugStep = std::make_shared<DebugStep>();

    // Set Rain texture
    m_rainTexture = new glow::Texture(GL_TEXTURE_2D);

    m_rainTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_rainTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_rainTexture->setParameter(GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    m_rainTexture->setParameter(GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    RawImage m_image("data/textures/rain/depth_full.raw", 1024*16, 768);

    m_rainTexture->image2D(0, GL_RGB8, 1024 * 16, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, m_image.rawData());
    m_rainTexture->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("Renderer", "rain"));
    glActiveTexture(GL_TEXTURE0);
    CheckGLError();
}

void Renderer::render(const CameraEx & camera)
{
    sceneStep(camera);
    handStep(camera);
    m_particleStep->draw(camera);
    m_shadowMappingStep->draw(camera);
    flushStep(camera);
}

void Renderer::takeScreenShot()
{
    m_takeScreenShot = true;
}

void Renderer::writeScreenShot()
{
    const int w = m_viewport.x;
    const int h = m_viewport.y;
    const int s = w * h * 3;

    static time_t lastTime = 0;
    static int n = 0;

    char * pixels = new char[s];
    if (m_takeScreenShot) {
        CheckGLError();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        CheckGLError();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glReadBuffer(GL_BACK);
        CheckGLError();
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        CheckGLError();

        time_t now = std::time(0);
        if (now == lastTime)
            ++n;
        else
            n = 0;
        lastTime = now;

        std::string fn = "screen_" + std::to_string(w) + "x" + std::to_string(h) + "_" + std::to_string(now) + "_" + std::to_string(n) + ".raw";
        glow::info("screenshot saved to %;", fn);
        std::ofstream file(fn, std::ios_base::trunc | std::ios_base::binary | std::ios_base::out);
        // flip the image
        for (int row = h - 1; row >= 0; --row) {
            file.write(&pixels[3*w*row], 3 * w);
        }
        file.close();
        m_takeScreenShot = false;
    }
    delete[] pixels;
}

void Renderer::sceneStep(const CameraEx & camera)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_fboByName.at("scene")->bind();

    glClearColor(0, 0, 0, 0);   // alpha == 0 means that the scene didn't set the color for a fragment position
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    World::instance()->terrain->draw(camera, { "bedrock" });

    m_fboByName.at("scene")->unbind();
}

void Renderer::handStep(const CameraEx & camera)
{
    m_fboByName.at("hand")->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    World::instance()->hand->draw(camera);

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
    m_quad->program()->setUniform("timef", int(World::instance()->getTime()));
    m_quad->program()->setUniform("view", camera.view());
    m_quad->program()->setUniform("camDirection", glm::normalize(camera.center() - camera.eye()));
    m_quad->program()->setUniform("viewport", camera.viewport());
    m_quad->program()->setUniform("viewProjectionInverse", camera.viewProjectionInvertedEx());
    m_quad->program()->setUniform("cameraPosition", camera.eye());
    m_quad->program()->setUniform("rainSampler", TextureManager::getTextureUnit("Renderer", "rain"));
    m_quad->program()->setUniform("rainStrength", World::instance()->rainStrength());
    m_quad->program()->setUniform("humidityFactor", World::instance()->humidityFactor);

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

void Renderer::toggleDrawHeatMap()
{
    m_drawHeatMap = !m_drawHeatMap;
    World::instance()->terrain->setDrawHeatMap(m_drawHeatMap);
}

void Renderer::resize(int width, int height)
{
    m_viewport.x = width; m_viewport.y = height;

    m_textureByName.at("sceneColor")->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_textureByName.at("sceneDepth")->image2D(0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fboByName.at("scene")->printStatus(true);

    m_textureByName.at("handColor")->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    m_textureByName.at("handDepth")->image2D(0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_fboByName.at("hand")->printStatus(true);

    for (auto step : m_steps) {
        assert(step);
        step->resize(width, height);
    }
}

void Renderer::initSkybox()
{
    const unsigned int skySize = 1120;
    RawImage * images[6];
    // http://www.3delyvisions.com/skf1.htm -> sky25.zip
    images[0] = new RawImage("data/textures/sky/left2moon.raw", skySize, skySize);
    images[1] = new RawImage("data/textures/sky/right2moon.raw", skySize, skySize);
    images[2] = new RawImage("data/textures/sky/top2moon.raw", skySize, skySize);
    images[3] = new RawImage("data/textures/sky/bot2moon.raw", skySize, skySize);
    images[4] = new RawImage("data/textures/sky/back2moon.raw", skySize, skySize);
    images[5] = new RawImage("data/textures/sky/front2moon.raw", skySize, skySize);

    m_skyboxTexture = new glow::Texture(GL_TEXTURE_CUBE_MAP);
    const int textureUnit = TextureManager::reserveTextureUnit("Renderer", "skybox");
    m_skyboxTexture->bindActive(GL_TEXTURE0 + textureUnit);
    
    for (int i = 0; i < 6; i++) {
        m_skyboxTexture->image2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGB8,
            skySize, skySize,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            images[i]->rawData());
    }

    m_skyboxTexture->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_skyboxTexture->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_skyboxTexture->setParameter(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    m_skyboxTexture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_skyboxTexture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    for (int i = 0; i < 6; ++i)
        delete images[i];
}
