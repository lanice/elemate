#pragma once

#include "renderingstep.h"

namespace glow {
    class VertexArrayObject;
    class Buffer;
    class Program;
}

class DebugStep : RenderingStep
{
public:
    virtual void draw(const CameraEx & camera) override;

protected:
    virtual void initialize();

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;

    glow::ref_ptr<glow::Program> m_wireframeBoxProgram;
    glow::ref_ptr<glow::Program> m_solidBoxProgram;
};
