#pragma once

#include <list>
#include <string>

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
    // for geometry drawing step

    glow::ref_ptr<glow::FrameBufferObject> m_depthFbo;
    glow::ref_ptr<glow::Texture> m_depthTex;

    // for postprocessing: use two texture buffers and swap them between the steps

    glow::ref_ptr<glow::Texture> m_postTexA;
    glow::ref_ptr<glow::Texture> m_postTexB;


    glow::ref_ptr<glow::Texture> m_normalsTex;

    // references to the result depth image
    glow::Texture * m_depthResultTex;

    class PostProcess {
    public:
        PostProcess(glow::Texture & source, glow::Texture * target, glow::Program & program);
        void draw();

        glow::Texture & m_source;
        glow::Texture * m_target;
        glow::ref_ptr<glow::FrameBufferObject> m_fbo;
        glow::ref_ptr<glow::Program> m_program;
        glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;

        void operator=(PostProcess&) = delete;
    };

    void addProcess(glow::Texture & source, glow::Texture * target, glow::Program & program);

    std::list<PostProcess> m_processes;    
};
