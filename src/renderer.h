#pragma once

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
    void particleWaterStep(const glowutils::Camera & camera);
    void flushStep();


    const World & m_world;

    void initialize();

    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;
    glow::ref_ptr<glow::Program> m_quadProgram;

    glow::ref_ptr<glow::FrameBufferObject> m_sceneFbo;
    glow::ref_ptr<glow::Texture> m_sceneColor;
    glow::ref_ptr<glow::Texture> m_sceneDepth;

    glow::ref_ptr<glow::FrameBufferObject> m_particleWaterFbo;
    glow::ref_ptr<glow::Texture> m_particleWaterColor;
    glow::ref_ptr<glow::Texture> m_particleWaterDepth;

public:
    Renderer() = delete;
    Renderer(Renderer&) = delete;
    void operator=(Renderer&) = delete;
};