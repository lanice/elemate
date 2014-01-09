#pragma once

#include "renderingstep.h"

class World;

class ShadowMappingStep : public RenderingStep
{
public:
    ShadowMappingStep(const World & world);

    virtual void draw(const glowutils::Camera & lightSource) override;
    virtual void resize(int width, int height) override;

    glow::Texture * result();

protected:
    const World & m_world;

    glow::ref_ptr<glow::FrameBufferObject> m_fbo;
    glow::ref_ptr<glow::Texture> m_tex;

public:
    void operator=(ShadowMappingStep&) = delete;
};
