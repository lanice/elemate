#pragma once

#include <glow/ref_ptr.h>

namespace glow {
    class FrameBufferObject;
    class Program;
    class Texture;
    class RenderBufferObject;
}
namespace glowutils {
    class ScreenAlignedQuad;
}
class CameraEx;

class RenderingStep
{
public:
    virtual ~RenderingStep();

    virtual void draw(const CameraEx & camera) = 0;
    virtual void resize(int width, int height) = 0;
};
