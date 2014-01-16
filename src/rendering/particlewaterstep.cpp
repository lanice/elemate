#include "particlewaterstep.h"

#include <cassert>

#include <glow/Texture.h>
#include <glow/Program.h>
#include <glow/FrameBufferObject.h>
#include <glowutils/File.h>
#include <glowutils/ScreenAlignedQuad.h>

#include "particledrawable.h"


ParticleWaterStep::ParticleWaterStep()
{
    m_depthTex = new glow::Texture(GL_TEXTURE_2D);
    m_depthTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_depthTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_depthTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_depthTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_depthTex->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);

    m_depthFbo = new glow::FrameBufferObject();
    m_depthFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_depthTex);
    m_depthFbo->setDrawBuffer(GL_NONE);
    m_depthFbo->unbind();


    m_blurredDepthTex = new glow::Texture(GL_TEXTURE_2D);
    m_blurredDepthTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_blurredDepthTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_blurredDepthTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_blurredDepthTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_blurredDepthFbo = new glow::FrameBufferObject();
    m_blurredDepthFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_blurredDepthTex);
    m_blurredDepthFbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_blurredDepthFbo->unbind();

    m_blurringProgram = new glow::Program();
    m_blurringProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particle_water_depthblurring.frag"));
    m_blurringProgram->setUniform("waterDepth", 0);
    m_blurringQuad = new glowutils::ScreenAlignedQuad(m_blurringProgram);


    m_normalsTex = new glow::Texture(GL_TEXTURE_2D);
    m_normalsTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_normalsTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_normalsTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_normalsTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_normalsFbo = new glow::FrameBufferObject();
    m_normalsFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_normalsTex);
    m_normalsFbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_normalsFbo->unbind();

    m_normalsProgram = new glow::Program();
    m_normalsProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particle_water_normals.frag"));
    m_normalsProgram->setUniform("waterDepth", 0);
    m_normalsQuad = new glowutils::ScreenAlignedQuad(m_normalsProgram);
}

void ParticleWaterStep::draw(const glowutils::Camera & camera)
{
    // render depth values to texture
    m_depthFbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    ParticleDrawable::drawParticles(camera);
    m_depthFbo->unbind();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // apply blurring on the depth image
    m_blurredDepthFbo->bind();
    m_depthTex->bind(GL_TEXTURE0);
    m_blurringQuad->draw();
    m_depthTex->unbind(GL_TEXTURE0);
    m_blurredDepthFbo->unbind();

    // calculate the surface normals
    m_normalsFbo->bind();
    m_blurredDepthTex->bind(GL_TEXTURE0);
    m_normalsQuad->draw();
    m_blurredDepthTex->unbind(GL_TEXTURE0);
    m_normalsFbo->unbind();
}

void ParticleWaterStep::resize(int width, int height)
{
    m_depthTex->image2D(0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_depthFbo->printStatus(true);
    assert(m_depthFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_blurredDepthTex->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    m_blurredDepthFbo->printStatus(true);
    assert(m_blurredDepthFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_normalsTex->image2D(0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    m_normalsFbo->printStatus(true);
    assert(m_normalsFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_blurringProgram->setUniform("viewport", glm::ivec2(width, height));
    m_normalsProgram->setUniform("viewport", glm::ivec2(width, height));
}

glow::Texture * ParticleWaterStep::normalsTex()
{
    assert(m_normalsTex);
    return m_normalsTex.get();
}

glow::Texture * ParticleWaterStep::depthTex()
{
    assert(m_blurredDepthTex);
    return m_blurredDepthTex.get();
}
