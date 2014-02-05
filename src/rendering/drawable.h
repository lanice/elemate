#pragma once

#include <glow/ref_ptr.h>
#include <glowutils/AxisAlignedBoundingBox.h>

namespace glow {
    class Program;
    class VertexArrayObject;
    class Buffer;
}

class CameraEx;

class Drawable
{
public:
    Drawable();
    virtual ~Drawable();

    virtual void draw(const CameraEx & camera);

    virtual const glowutils::AxisAlignedBoundingBox & boundingBox() const;
    virtual void setBoudingBox(const glowutils::AxisAlignedBoundingBox & bbox);

protected:
    virtual void drawImplementation(const CameraEx & camera) = 0;

    virtual void initialize();

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;

    glowutils::AxisAlignedBoundingBox m_bbox;
};
