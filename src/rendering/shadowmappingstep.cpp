#include "shadowmappingstep.h"

namespace glow {
    class Buffer; // missing forward declaration in FrameBufferObject.h
}
#include <glow/DebugMessageOutput.h> // for error handling of missing glTexParameterfv
#include <glow/FrameBufferObject.h>
#include <glow/Texture.h>
#include <glow/RenderBufferObject.h>

#include <glm/gtc/matrix_transform.hpp>

#include "world.h"
#include "terrain/terrain.h"
#include "hand.h"
#include "cameraex.h"

#undef far  // that's for windows (minwindef.h)

ShadowMappingStep::ShadowMappingStep(const World & world)
: m_world(world)
, m_lightCam(new CameraEx())
{
    m_lightTex = new glow::Texture(GL_TEXTURE_2D);
    m_lightTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_lightTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_lightTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_lightTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_lightTex->image2D(0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    m_lightFbo = new glow::FrameBufferObject();
    m_lightFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_lightTex);
    m_lightFbo->setDrawBuffers({ GL_NONE });
    m_lightFbo->printStatus(true);
    assert(m_lightFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);
    m_lightFbo->unbind();


    m_shadowTex = new glow::Texture(GL_TEXTURE_2D);
    m_shadowTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_shadowTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_shadowTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_shadowTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_shadowDepthBuffer = new glow::RenderBufferObject();

    m_shadowFbo = new glow::FrameBufferObject();
    m_shadowFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_shadowTex);
    m_shadowFbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_shadowDepthBuffer);
    m_shadowFbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_shadowFbo->unbind();


    const TerrainSettings & ts = m_world.terrain->settings;
    const float right = ts.sizeX * 0.5f;
    const float top = ts.maxHeight;
    const float far = ts.sizeZ * 0.5f;

    m_lightCam->setEye(glm::vec3(0, 0, 0));
    m_lightCam->setUp(glm::vec3(0, 1, 0));
    m_lightCam->setLeft(-right);
    m_lightCam->setRight(right);
    m_lightCam->setTop(top);
    m_lightCam->setBottom(-top);
    m_lightCam->setZFar(far);
    m_lightCam->setZNearOtho(-far);
}

void ShadowMappingStep::drawLightMap(const glowutils::Camera & camera)
{
    m_lightCam->setCenter(-m_world.sunlightInvDirection());

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glViewport(0, 0, 1024, 1024);

    // draw the scene into the light map
    m_lightFbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    m_world.terrain->drawLightMap(*m_lightCam);
    m_world.hand->drawLightMap(*m_lightCam);

    m_lightFbo->unbind();

    glViewport(0, 0, camera.viewport().x, camera.viewport().y);
}

void ShadowMappingStep::draw(const glowutils::Camera & camera)
{
    drawLightMap(camera);

    m_lightTex->bind(GL_TEXTURE0);
    m_shadowFbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_world.terrain->drawShadowMapping(camera, *m_lightCam);
    m_world.hand->drawShadowMapping(camera, *m_lightCam);

    m_shadowFbo->unbind();
    m_lightTex->unbind(GL_TEXTURE0);
}

void ShadowMappingStep::resize(int width, int height)
{
    m_shadowTex->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    m_shadowDepthBuffer->storage(GL_DEPTH_COMPONENT32F, width, height);
    m_shadowFbo->printStatus(true);
    assert(m_shadowFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);
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
