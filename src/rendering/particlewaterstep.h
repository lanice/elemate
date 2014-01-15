#pragma once

#include "renderingstep.h"

class ParticleWaterStep : public RenderingStep
{
public:
    ParticleWaterStep();

    virtual void draw(const glowutils::Camera & camera) override;
    virtual void resize(int width, int height) override;

    glow::Texture * normalsTex();
    glow::Texture * depthTex();

protected:
    glow::ref_ptr<glow::FrameBufferObject> m_depthFbo;
    glow::ref_ptr<glow::Texture> m_depthTex;

    glow::ref_ptr<glow::FrameBufferObject> m_blurredDepthFbo;
    glow::ref_ptr<glow::Texture> m_blurredDepthTex;
    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_blurringQuad;
    glow::ref_ptr<glow::Program> m_blurringProgram;

    glow::ref_ptr<glow::FrameBufferObject> m_normalsFbo;
    glow::ref_ptr<glow::Texture> m_normalsTex;
    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_normalsQuad;
    glow::ref_ptr<glow::Program> m_normalsProgram;
};
