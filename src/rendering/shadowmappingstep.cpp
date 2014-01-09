#include "shadowmappingstep.h"

namespace glow {
    class Buffer; // missing forward declaration in FrameBufferObject.h
}
#include <glow/FrameBufferObject.h>
#include <glow/Texture.h>
#include <glow/RenderBufferObject.h>
#include <glowutils/Camera.h>

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

    m_lightFbo = new glow::FrameBufferObject();
    m_lightFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_lightTex);
    m_lightFbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_depthBuffer);
    m_lightFbo->setDrawBuffer({ GL_COLOR_ATTACHMENT0 });
    m_lightFbo->unbind();


    m_shadowTex = new glow::Texture(GL_TEXTURE_2D);
    m_shadowTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_shadowTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_shadowTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_shadowTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_shadowFbo = new glow::FrameBufferObject();
    m_shadowFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_shadowTex);
    m_shadowFbo->attachRenderBuffer(GL_DEPTH_ATTACHMENT, m_depthBuffer);
    m_shadowFbo->setDrawBuffer({ GL_COLOR_ATTACHMENT0 });
    m_shadowFbo->unbind();
}

void ShadowMappingStep::draw(const glowutils::Camera & camera)
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // draw the scene into the light map
    m_lightFbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: get the global sunlight defintion
    static glowutils::Camera lightCam(camera);
    lightCam.setEye(glm::vec3(5.0f, 10.0f, 0.0f));
    lightCam.setCenter(glm::vec3(0.0f, 0.0f, 0.0f));

    m_world.terrain->drawLightMap(lightCam);

    m_lightFbo->unbind();

    // create the shadow map

    m_lightTex->bind(GL_TEXTURE0);
    m_shadowFbo->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_world.terrain->drawShadowMapping(camera, lightCam);

    m_shadowFbo->unbind();
    m_lightTex->unbind(GL_TEXTURE0);
}

void ShadowMappingStep::resize(int width, int height)
{
    m_lightTex->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    m_depthBuffer->storage(GL_DEPTH_COMPONENT32F, width, height);
    m_lightFbo->printStatus(true);
    assert(m_lightFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_shadowTex->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    m_shadowFbo->printStatus(true);
    assert(m_shadowFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);
}

glow::Texture * ShadowMappingStep::result()
{
    assert(m_shadowTex);
    return m_shadowTex.get();
}
