#pragma once

#include "renderingstep.h"

#include <vector>

#include <glm/glm.hpp>

#include <glow/ref_ptr.h>

namespace glow {
    class Texture;
    class FrameBufferObject;
    class RenderBufferObject;
    class Program;
}

class ShadowMappingStep : public RenderingStep
{
public:
    ShadowMappingStep();

    /** Draw the depth values as seen from the sun light defined in the world (orthographic projection).
        Draw the shadow mapping for this light source, as seen by the passed camera. */
    virtual void draw(const CameraEx & camera) override;
    virtual void resize(int width, int height) override;

    glow::Texture * lightMap();
    glow::Texture * result();

    static void setUniforms(glow::Program & program);

    static const glm::mat4 s_biasMatrix;

protected:
    CameraEx * m_lightCam;

    virtual void drawLightMap(const CameraEx & camera);

    virtual void calculateLightMatrix(const CameraEx & camera);

    glow::ref_ptr<glow::FrameBufferObject> m_lightFbo;
    glow::ref_ptr<glow::Texture> m_lightTex;

    glow::ref_ptr<glow::FrameBufferObject> m_shadowFbo;
    glow::ref_ptr<glow::RenderBufferObject> m_shadowDepthBuffer;
    glow::ref_ptr<glow::Texture> m_shadowTex;

    static std::vector<glm::vec2> * s_depthSamples;
    static std::vector<glm::vec2> * s_earlyBailSamples;

public:
    void operator=(ShadowMappingStep&) = delete;
};
