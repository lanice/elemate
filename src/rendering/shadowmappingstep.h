#pragma once

#include <vector>

#include <glow/Array.h>

#include <glm/glm.hpp>

#include "renderingstep.h"

class World;
class CameraEx;

class ShadowMappingStep : public RenderingStep
{
public:
    ShadowMappingStep(const World & world);

    virtual void draw(const glowutils::Camera & camera) override;
    virtual void resize(int width, int height) override;

    glow::Texture * lightMap();
    glow::Texture * result();

    static void setUniforms(glow::Program & program);

    static const glm::mat4 s_biasMatrix;

protected:
    const World & m_world;

    CameraEx * m_lightCam;

    virtual void drawLightMap(const glowutils::Camera & camera);

    glow::ref_ptr<glow::FrameBufferObject> m_lightFbo;
    glow::ref_ptr<glow::Texture> m_lightTex;

    glow::ref_ptr<glow::FrameBufferObject> m_shadowFbo;
    glow::ref_ptr<glow::RenderBufferObject> m_shadowDepthBuffer;
    glow::ref_ptr<glow::Texture> m_shadowTex;

    static const glow::Vec2Array s_depthSamples;
    static const glow::Vec2Array s_earlyBailSamples;
    static const GLuint s_lightmapSlot;

public:
    void operator=(ShadowMappingStep&) = delete;
};
