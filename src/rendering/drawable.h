#pragma once

#include <glow/ref_ptr.h>

namespace glow {
    class Program;
    class VertexArrayObject;
    class Buffer;
}
class CameraEx;
class World;

class Drawable
{
public:
    Drawable(const World & world);
    virtual ~Drawable();

    virtual void draw(const CameraEx & camera);
       /** writes the linearized depth into the current depth attachment */
    virtual void drawDepthMap(const CameraEx & camera);
    virtual void drawShadowMapping(const CameraEx & camera, const CameraEx & lightSource);

protected:
    const World & m_world;

    virtual void drawImplementation(const CameraEx & camera) = 0;
    virtual void drawDepthMapImpl(const CameraEx & camera) = 0;
    virtual void drawShadowMappingImpl(const CameraEx & camera, const CameraEx & lightSource) = 0;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_indexBuffer;
    glow::ref_ptr<glow::Buffer> m_vbo;

    virtual void initialize();

    // Shadowing
    glow::ref_ptr<glow::Program> m_depthMapProgram;
    glow::ref_ptr<glow::Program> m_depthMapLinearizedProgram;
    glow::ref_ptr<glow::Program> m_shadowMappingProgram;
    virtual void initDepthMapProgram() = 0;
    virtual void initShadowMappingProgram() = 0;

public:
    void operator=(Drawable&) = delete;
};
