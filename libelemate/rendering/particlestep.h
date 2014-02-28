#pragma once

#include <list>
#include <string>

#include "renderingstep.h"

class ParticleStep : public RenderingStep
{
public:
    ParticleStep();

    virtual void draw(const CameraEx & camera) override;
    virtual void resize(int width, int height) override;

protected:
    // for geometry drawing step

    glow::ref_ptr<glow::FrameBufferObject> m_particleSceneFbo;
    glow::ref_ptr<glow::Texture> m_depthTex;
    glow::ref_ptr<glow::Texture> m_elementIdTex;

    glow::ref_ptr<glow::Texture> m_postTempTex;


    glow::ref_ptr<glow::Texture> m_normalsTex;

    // references to the result depth image
    glow::Texture * m_depthResultTex;

    class PostProcess {
    public:
        PostProcess(int sourceTexUnit, glow::Texture & target, glow::Program & program);
        void draw();

        int m_sourceTexUnit;
        glow::Texture * m_target;
        glow::ref_ptr<glow::FrameBufferObject> m_fbo;
        glow::ref_ptr<glow::Program> m_program;
        glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;

        void operator=(PostProcess&) = delete;
    };

    void addProcess(int sourceTexUnit, glow::Texture & target, glow::Program & program);

    std::list<PostProcess> m_processes;    
};
