#pragma once

#include <glow/ref_ptr.h>

namespace glow {
    class Program;
    class VertexArrayObject;
    class Buffer;
}
namespace glowutils {
    class Camera;
}
class CameraEx;
class World;

class Drawable
{
public:
    Drawable(const World & world);
    virtual ~Drawable();

    virtual void draw(const glowutils::Camera & camera);
       /** writes the linearized depth into the current depth attachment */
    virtual void drawDepthMap(const CameraEx & camera);
    virtual void drawShadowMapping(const glowutils::Camera & camera, const CameraEx & lightSource);

protected:
    const World & m_world;

    virtual void drawImplementation(const glowutils::Camera & camera) = 0;
    virtual void drawDepthMapImpl(const CameraEx & camera) = 0;
    virtual void drawShadowMappingImpl(const glowutils::Camera & camera, const CameraEx & lightSource) = 0;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_indexBuffer;
    glow::ref_ptr<glow::Buffer> m_vbo;

    virtual void initialize();

    // Shadowing
    glow::ref_ptr<glow::Program> m_depthMapProgram;
    glow::ref_ptr<glow::Program> m_shadowMappingProgram;
    virtual void initDepthMapProgram() = 0;
    virtual void initShadowMappingProgram() = 0;

public:
    void operator=(Drawable&) = delete;
};
