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

protected:
    // drawing steps
    void colorStep(const glowutils::Camera & camera);
    void flushStep();


    const World & m_world;

    void initialize();

    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;
    glow::ref_ptr<glow::Program> m_quadProgram;

    glow::ref_ptr<glow::FrameBufferObject> m_colorFbo;

    glow::ref_ptr<glow::Texture> m_colorTex;
    glow::ref_ptr<glow::RenderBufferObject> m_depthBuffer;

public:
    Renderer() = delete;
    Renderer(Renderer&) = delete;
    void operator=(Renderer&) = delete;
};
