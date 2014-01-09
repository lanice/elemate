#pragma once

#include <memory>

#include <glow/ref_ptr.h>

namespace glow {
    class FrameBufferObject;
    class Texture;
    class RenderBufferObject;
    class Program;
}
namespace glowutils {
    class Camera;
    class ScreenAlignedQuad;
}
class World;
class ParticleWaterStep;

class Renderer
{
public:
    Renderer(const World & world);

    void operator()(const glowutils::Camera & camera);

    void resize(int width, int height);

    const glow::FrameBufferObject * sceneFbo() const;

protected:
    // drawing steps
    void sceneStep(const glowutils::Camera & camera);
    void handStep(const glowutils::Camera & camera);
    std::shared_ptr<ParticleWaterStep> m_particleWaterStep;
    void shadowStep(const glowutils::Camera & lightSource);
    void flushStep();


    const World & m_world;

    void initialize();

    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;
    glow::ref_ptr<glow::Program> m_quadProgram;

    glow::ref_ptr<glow::FrameBufferObject> m_sceneFbo;
    glow::ref_ptr<glow::Texture> m_sceneColor;
    glow::ref_ptr<glow::Texture> m_sceneDepth;

    glow::ref_ptr<glow::FrameBufferObject> m_handFbo;
    glow::ref_ptr<glow::RenderBufferObject> m_handDepth;


    glow::ref_ptr<glow::FrameBufferObject> m_shadowFbo;
    glow::ref_ptr<glow::Texture> m_shadowDepthTex;
public:
    Renderer() = delete;
    Renderer(Renderer&) = delete;
    void operator=(Renderer&) = delete;
};
