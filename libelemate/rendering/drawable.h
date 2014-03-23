#pragma once

#include <set>

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

    /** basic draw call that calls the lazy initialize() and the drawImplementation() with bound vertex array object */
    virtual void draw(const CameraEx & camera);

    /** @return the axis aligned bounding box of the drawable. May be specified by subclasses */
    virtual const glowutils::AxisAlignedBoundingBox & boundingBox() const;

    virtual const glm::mat4 & transform() const;

protected:
    /** The DebugStep is a friend to fetch debug information from all drawable instances. */
    friend class DebugStep;

    static std::set<Drawable*> s_drawableInstances;

    /** draw call to be implemented by subclasses */
    virtual void drawImplementation(const CameraEx & camera) = 0;

    /** Subclasses should basically create the vertex array object here. */
    virtual void initialize() = 0;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;

    glowutils::AxisAlignedBoundingBox m_bbox;
};
