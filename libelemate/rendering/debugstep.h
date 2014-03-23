#pragma once

#include "renderingstep.h"

#include <glow/ref_ptr.h>

namespace glow {
    class VertexArrayObject;
    class Buffer;
    class Program;
}

/** Draw some debug information for registered classes. */
class DebugStep : public RenderingStep
{
public:
    /** draw bounding boxes of Drawable subclasses and the particle collision volumes */
    virtual void draw(const CameraEx & camera) override;

protected:
    virtual void initialize();

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;

    glow::ref_ptr<glow::Program> m_wireframeBoxProgram;
    glow::ref_ptr<glow::Program> m_solidBoxProgram;
};
