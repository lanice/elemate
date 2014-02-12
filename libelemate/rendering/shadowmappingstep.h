#pragma once

#include <vector>

#include <glow/global.h>

#include <glm/glm.hpp>

#include "renderingstep.h"

class World;

class ShadowMappingStep : public RenderingStep
{
public:
    ShadowMappingStep(const World & world);

    virtual void draw(const CameraEx & camera) override;
    virtual void resize(int width, int height) override;

    glow::Texture * lightMap();
    glow::Texture * result();

    static void setUniforms(glow::Program & program);

    static const glm::mat4 s_biasMatrix;

protected:
    const World & m_world;

    CameraEx * m_lightCam;

    virtual void drawLightMap(const CameraEx & camera);

    virtual void calculateLightMatrix(const CameraEx & camera);

    glow::ref_ptr<glow::FrameBufferObject> m_lightFbo;
    glow::ref_ptr<glow::Texture> m_lightTex;

    glow::ref_ptr<glow::FrameBufferObject> m_shadowFbo;
    glow::ref_ptr<glow::RenderBufferObject> m_shadowDepthBuffer;
    glow::ref_ptr<glow::Texture> m_shadowTex;

    static const std::vector<glm::vec2> s_depthSamples;
    static const std::vector<glm::vec2> s_earlyBailSamples;
    static const GLint s_lightmapSlot;

public:
    void operator=(ShadowMappingStep&) = delete;
};
