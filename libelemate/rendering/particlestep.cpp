#include "particlestep.h"

#include <cassert>

#include <glow/Texture.h>
#include <glow/Program.h>
#include <glow/FrameBufferObject.h>
#include <glowutils/global.h>
#include <glowutils/ScreenAlignedQuad.h>
#include "utils/cameraex.h"

#include "particledrawable.h"
#include "world.h"
#include "terrain/terrain.h"
#include "texturemanager.h"

ParticleStep::ParticleStep()
{
    // first step: get depth image
    m_depthTex = new glow::Texture(GL_TEXTURE_2D);
    m_depthTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_depthTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_depthTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_depthTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_depthTex->setParameter(GL_TEXTURE_COMPARE_MODE, GL_NONE);
    m_depthTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("ParticleStep", "sphereDepth"));
    glActiveTexture(GL_TEXTURE0);

    m_elementIdTex = new glow::Texture(GL_TEXTURE_2D);
    m_elementIdTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_elementIdTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_elementIdTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_elementIdTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_elementIdTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("ParticleStep", "elementID"));
    glActiveTexture(GL_TEXTURE0);

    m_particleSceneFbo = new glow::FrameBufferObject();
    m_particleSceneFbo->attachTexture2D(GL_COLOR_ATTACHMENT0, m_elementIdTex);
    m_particleSceneFbo->attachTexture2D(GL_DEPTH_ATTACHMENT, m_depthTex);
    m_particleSceneFbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_particleSceneFbo->unbind();


    m_postTempTex = new glow::Texture(GL_TEXTURE_2D);
    m_postTempTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_postTempTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_postTempTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_postTempTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_postTempTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("ParticleStep", "postTemp"));
    glActiveTexture(GL_TEXTURE0);

    m_depthResultTex = new glow::Texture(GL_TEXTURE_2D);
    m_depthResultTex->setParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    m_depthResultTex->setParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    m_depthResultTex->setParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_depthResultTex->setParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_depthResultTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("ParticleStep", "particleDepth"));
    glActiveTexture(GL_TEXTURE0);

    // postprocessing: 2x depth blurring

    glow::Program * blurHorizontalProgram = new glow::Program();
    blurHorizontalProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particles/particle_depthblurring_v.frag"));
    addProcess(TextureManager::getTextureUnit("ParticleStep", "sphereDepth"), *m_postTempTex, *blurHorizontalProgram);

    glow::Program * blurVerticalProgram = new glow::Program();
    blurVerticalProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particles/particle_depthblurring_h.frag"));
    addProcess(TextureManager::getTextureUnit("ParticleStep", "postTemp"), *m_depthResultTex, *blurVerticalProgram);

    std::vector<float> binomCoeff;
    std::vector<int32_t> binomOffset;

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
    m_normalsTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit("ParticleStep", "particleNormals"));
    glActiveTexture(GL_TEXTURE0);

    glow::Program * normalProgram = new glow::Program();
    normalProgram->attach(
        World::instance()->sharedShader(GL_VERTEX_SHADER, "shader/flush.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/particles/particle_normals.frag"));
    addProcess(TextureManager::getTextureUnit("ParticleStep", "particleDepth"), *m_normalsTex, *normalProgram);
}

ParticleStep::PostProcess::PostProcess(int sourceTexUnit, glow::Texture & target, glow::Program & program)
: m_sourceTexUnit(sourceTexUnit)
, m_target(nullptr)
, m_fbo(nullptr)
, m_program(&program)
, m_quad(new glowutils::ScreenAlignedQuad(&program))
{
    m_quad->setSamplerUniform(sourceTexUnit);

    m_fbo = new glow::FrameBufferObject;
    m_fbo->attachTexture2D(GL_COLOR_ATTACHMENT0, &target);
    m_fbo->setDrawBuffers({ GL_COLOR_ATTACHMENT0 });
    m_fbo->unbind();
}

void ParticleStep::PostProcess::draw()
{
    if (m_fbo)
        m_fbo->bind();

    m_quad->draw();

    if (m_fbo)
        m_fbo->unbind();
}

void ParticleStep::addProcess(int sourceTexUnit, glow::Texture & target, glow::Program & program)
{
    m_processes.push_back(PostProcess(sourceTexUnit, target, program));
}

void ParticleStep::draw(const CameraEx & camera)
{
    // render depth values to texture
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    m_particleSceneFbo->bind();
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ParticleDrawable::drawParticles(camera);
    World::instance()->terrain->drawDepthMap(camera, { "water" });
    m_particleSceneFbo->unbind();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // apply post processing on depth image, calculate normals
    for (PostProcess & process : m_processes) {
        process.draw();
    }
}

void ParticleStep::resize(int width, int height)
{
    m_elementIdTex->image2D(0, GL_R8UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
    m_depthTex->image2D(0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
    m_particleSceneFbo->printStatus(true);

    m_postTempTex->image2D(0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, nullptr);
    m_depthResultTex->image2D(0, GL_R16, width, height, 0, GL_RED, GL_UNSIGNED_SHORT, nullptr);
    m_normalsTex->image2D(0, GL_RGB16, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT, nullptr);

    for (PostProcess & process : m_processes) {
        process.m_fbo->printStatus(true);

        process.m_program->setUniform("viewport", glm::ivec2(width, height));
    }
}
