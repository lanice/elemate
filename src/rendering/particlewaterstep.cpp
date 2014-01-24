#include "particlewaterstep.h"

#include <cassert>

#include <glow/Texture.h>
#include <glow/Program.h>
#include <glow/FrameBufferObject.h>
#include <glowutils/File.h>
#include <glowutils/ScreenAlignedQuad.h>
#include "cameraex.h"

#include "particledrawable.h"
#include "world.h"
#include "terrain/terrain.h"


ParticleWaterStep::ParticleWaterStep()
{
    // first step: get depth image
    m_depthTex = new glow::Texture(GL_TEXTURE_2D);
    m_depthTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_depthTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_depthTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_depthTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_depthTex->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);

    m_depthFbo = new glow::FrameBufferObject();
    m_depthFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_depthTex);
    m_depthFbo->setDrawBuffers({ GL_NONE });
    m_depthFbo->unbind();


    m_postTexA = new glow::Texture(GL_TEXTURE_2D);
    m_postTexA->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_postTexA->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_postTexA->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_postTexA->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_postTexB = new glow::Texture(GL_TEXTURE_2D);
    m_postTexB->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_postTexB->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_postTexB->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_postTexB->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // postprocessing: 2x depth blurring

    glow::Program * blurHorizontalProgram = new glow::Program();
    blurHorizontalProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particle_water_depthblurring_h.frag"));
    addProcess(*m_depthTex, m_postTexA, *blurHorizontalProgram);

    glow::Program * blurVerticalProgram = new glow::Program();
    blurVerticalProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particle_water_depthblurring_v.frag"));
    addProcess(*m_postTexA, m_postTexB, *blurVerticalProgram);

    m_depthResultTex = m_postTexB;

    glow::FloatArray binomCoeff;
    glow::IntArray binomOffset;

    for (int nHalf = 0; nHalf < 30; ++nHalf){
        binomOffset.push_back(static_cast<int>(binomCoeff.size()));
        for (int k = nHalf; k >= 0; --k){
            double num = 1.0;
            for (int i = 0; i < 2 * nHalf - k; i++)
                num = 0.5*num*(2 * nHalf - i) / (i + 1);
            for (int i = 0; i < k; i++)
                num /= 2;
            binomCoeff.push_back(float(num));
        }
    }

    blurVerticalProgram->setUniform("binomCoeff", binomCoeff);
    blurVerticalProgram->setUniform("binomOffset", binomOffset);
    blurHorizontalProgram->setUniform("binomCoeff", binomCoeff);
    blurHorizontalProgram->setUniform("binomOffset", binomOffset);


    // postprocessing: calculate normals from depth image
    
    m_normalsTex = new glow::Texture(GL_TEXTURE_2D);
    m_normalsTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_normalsTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_normalsTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_normalsTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glow::Program * normalProgram = new glow::Program();
    normalProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particle_water_normals.frag"));
    addProcess(*m_postTexB, m_normalsTex, *normalProgram);
}

ParticleWaterStep::PostProcess::PostProcess(glow::Texture & source, glow::Texture * target, glow::Program & program)
: m_source(source)
, m_target(target)
, m_fbo(nullptr)
, m_program(&program)
, m_quad(new glowutils::ScreenAlignedQuad(&program))
{
    if (m_target) {
        m_fbo = new glow::FrameBufferObject;
        m_fbo->attachTexture2D(GL_COLOR_ATTACHMENT0, target);
        m_fbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
        m_fbo->unbind();
    }
}

void ParticleWaterStep::PostProcess::draw()
{
    if (m_fbo)
        m_fbo->bind();

    m_source.bind(GL_TEXTURE0);

    m_quad->draw();

    m_source.unbind(GL_TEXTURE0);

    if (m_fbo)
        m_fbo->unbind();
}

void ParticleWaterStep::addProcess(glow::Texture & source, glow::Texture * target, glow::Program & program)
{
    m_processes.push_back(PostProcess(source, target, program));
}

void ParticleWaterStep::draw(const CameraEx & camera)
{
    // render depth values to texture
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_depthFbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    ParticleDrawable::drawParticles(camera);
    World::instance()->terrain->drawDepthMap(camera, { "water" });
    m_depthFbo->unbind();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // apply post processing on depth image, calculate normals
    for (PostProcess & process : m_processes) {
        process.draw();
    }
}

void ParticleWaterStep::resize(int width, int height)
{
    m_depthTex->image2D(0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    m_depthFbo->printStatus(true);
    assert(m_depthFbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

    m_postTexA->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    m_postTexB->image2D(0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);

    m_normalsTex->image2D(0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

    for (PostProcess & process : m_processes) {
        process.m_fbo->printStatus(true);
        assert(process.m_fbo->checkStatus() == GL_FRAMEBUFFER_COMPLETE);

        process.m_program->setUniform("viewport", glm::ivec2(width, height));
    }
}

glow::Texture * ParticleWaterStep::depthTex()
{
    assert(m_depthResultTex);
    return m_depthResultTex;
}

glow::Texture * ParticleWaterStep::normalsTex()
{
    assert(m_normalsTex);
    return m_normalsTex;
}
