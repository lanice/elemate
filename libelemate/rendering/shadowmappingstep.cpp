#include "shadowmappingstep.h"

namespace glow {
    class Buffer; // missing forward declaration in FrameBufferObject.h
}
#include <glow/FrameBufferObject.h>
#include <glow/Texture.h>
#include <glow/RenderBufferObject.h>
#include <glow/Program.h>
#include <glowutils/AxisAlignedBoundingBox.h>
#include "utils/cameraex.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/random.hpp>

#include "world.h"
#include "terrain/terrain.h"
#include "ui/hand.h"
#include "texturemanager.h"

#undef far  // that's for windows (minwindef.h)

const glm::mat4 ShadowMappingStep::s_biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0);

std::vector<glm::vec2> * initDepthSamples() {
    std::vector<glm::vec2> * samples = new std::vector<glm::vec2>;
    for (int i = 0; i < 32; ++i)
        samples->push_back(glm::vec2(glm::linearRand(-1.0f, 1.0f), glm::linearRand(-1.0f, 1.0f)));
    return samples;
}
std::vector<glm::vec2> * ShadowMappingStep::s_depthSamples = nullptr;
static const float earlyBailDistance = 3.0f;
std::vector<glm::vec2> * ShadowMappingStep::s_earlyBailSamples = nullptr;


ShadowMappingStep::ShadowMappingStep()
: m_lightCam(new CameraEx(ProjectionType::orthographic))
{
    m_lightCam->setUp(glm::vec3(0, 1, 0));
    m_lightCam->setTop(World::instance()->terrain->settings.maxHeight);
    m_lightCam->setBottom(-World::instance()->terrain->settings.maxHeight);
    m_lightCam->setViewport(2048, 2048);

    m_lightTex = new glow::Texture(GL_TEXTURE_2D);
    m_lightTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    m_lightTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_lightTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    m_lightTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float lightBorderColor[4] = { 1, 1, 1, 1 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, lightBorderColor);
    CheckGLError();
    m_lightTex->image2D(0, GL_DEPTH_COMPONENT24, m_lightCam->viewport().x, m_lightCam->viewport().y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_lightTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("ShadowMapping", "lightMap"));
    glActiveTexture(GL_TEXTURE0);

    m_lightFbo = new glow::FrameBufferObject();
    m_lightFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_lightTex);
    m_lightFbo->setDrawBuffers({ GL_NONE });
    m_lightFbo->printStatus(true);
    m_lightFbo->unbind();


    m_shadowTex = new glow::Texture(GL_TEXTURE_2D);
    m_shadowTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_shadowTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_shadowTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_shadowTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_shadowTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("ShadowMapping", "shadowMap"));
    glActiveTexture(GL_TEXTURE0);

    m_shadowDepthBuffer = new glow::RenderBufferObject();

    m_shadowFbo = new glow::FrameBufferObject();
    m_shadowFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_shadowTex);
    m_shadowFbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_shadowDepthBuffer);
    m_shadowFbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_shadowFbo->unbind();
}

void ShadowMappingStep::calculateLightMatrix(const CameraEx & camera)
{
    m_lightCam->setEye(glm::vec3(camera.eye().x, 0.0f, camera.eye().z));
    m_lightCam->setCenter(m_lightCam->eye() - World::instance()->sunPosition());
    const float shadowWidth = camera.zFarEx();
    m_lightCam->setLeft(-shadowWidth*1.5f);
    m_lightCam->setRight(shadowWidth*1.5f);
    m_lightCam->setZFarEx(shadowWidth);
    m_lightCam->setZNearEx(-shadowWidth);
}

void ShadowMappingStep::drawLightMap(const CameraEx & camera)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    calculateLightMatrix(camera);

    glViewport(0, 0, m_lightCam->viewport().x, m_lightCam->viewport().y);

    // draw the scene into the light map
    m_lightFbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    World::instance()->terrain->drawDepthMap(*m_lightCam, { "bedrock" });
    World::instance()->hand->drawDepthMap(*m_lightCam);

    m_lightFbo->unbind();

    glViewport(0, 0, camera.viewport().x, camera.viewport().y);
}

void ShadowMappingStep::draw(const CameraEx & camera)
{
    drawLightMap(camera);

    m_shadowFbo->bind();
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    World::instance()->terrain->drawShadowMapping(camera, *m_lightCam, { "bedrock" });
    World::instance()->hand->drawShadowMapping(camera, *m_lightCam);

    m_shadowFbo->unbind();
}

void ShadowMappingStep::resize(int width, int height)
{
    m_shadowTex->image2D(0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, nullptr);
    m_shadowDepthBuffer->storage(GL_DEPTH_COMPONENT16, width, height);
    m_shadowFbo->printStatus(true);
}

glow::Texture * ShadowMappingStep::lightMap()
{
    assert(m_lightTex);
    return m_lightTex.get();
}

glow::Texture * ShadowMappingStep::result()
{
    assert(m_shadowTex);
    return m_shadowTex.get();
}

void ShadowMappingStep::setUniforms(glow::Program & program)
{
    if (!s_depthSamples)
    {
        ShadowMappingStep::s_depthSamples = initDepthSamples();
        ShadowMappingStep::s_earlyBailSamples = new std::vector<glm::vec2>(
        {
            glm::vec2(0, 0),
                glm::vec2(earlyBailDistance, earlyBailDistance),
                glm::vec2(earlyBailDistance, -earlyBailDistance),
                glm::vec2(-earlyBailDistance, earlyBailDistance),
                glm::vec2(-earlyBailDistance, -earlyBailDistance)
        });
    }

    program.setUniform("lightMap", TextureManager::getTextureUnit("ShadowMapping", "lightMap"));
    program.setUniform("depthSamples", *s_depthSamples);
    program.setUniform("earlyBailSamples", *s_earlyBailSamples);
}
