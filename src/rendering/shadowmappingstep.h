#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "renderingstep.h"

class World;

class ShadowMappingStep : public RenderingStep
{
public:
    ShadowMappingStep(const World & world);

    virtual void draw(const glowutils::Camera & camera) override;
    virtual void resize(int width, int height) override;

    glow::Texture * lightMap();
    glow::Texture * result();

    struct Definitions {
        std::vector<glm::vec2> shadowSamples;
        glow::ref_ptr<glowutils::Camera> lightSource;
        float lightSize;
        float searchWidth;
        float zOffset;
    };

protected:
    const World & m_world;

    virtual void drawLightMap(const glowutils::Camera & camera);

    glow::ref_ptr<glow::FrameBufferObject> m_lightFbo;
    glow::ref_ptr<glow::Texture> m_lightTex;

    glow::ref_ptr<glow::FrameBufferObject> m_shadowFbo;
    glow::ref_ptr<glow::RenderBufferObject> m_shadowDepthBuffer;
    glow::ref_ptr<glow::Texture> m_shadowTex;

public:
    void operator=(ShadowMappingStep&) = delete;
};
