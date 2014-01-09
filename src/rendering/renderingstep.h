#pragma once

#include <glow/ref_ptr.h>

namespace glow {
    class FrameBufferObject;
    class Program;
    class Texture;
}
namespace glowutils {
    class Camera;
    class ScreenAlignedQuad;
}

class RenderingStep
{
public:
    virtual ~RenderingStep();

    virtual void draw(const glowutils::Camera & camera) = 0;
    virtual void resize(int width, int height) = 0;
};
