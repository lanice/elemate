#pragma once

#include "drawable.h"

namespace glow {
    class Program;
}

/** Superclass for drawables that are meant to cast and receive shadows. */
class ShadowingDrawable : public Drawable
{
public:
    ShadowingDrawable();

    /** Writes the depth into the current depth attachment.
        Depth values will be linearized for perspective projections. */
    virtual void drawDepthMap(const CameraEx & camera);
    /** Draws the shadow map, that can be used in the flush shader to be combined with the color image of the rendered scene. */
    virtual void drawShadowMapping(const CameraEx & camera, const CameraEx & lightSource);

protected:
    virtual void drawDepthMapImpl(const CameraEx & camera) = 0;
    virtual void drawShadowMappingImpl(const CameraEx & camera, const CameraEx & lightSource) = 0;

    glow::ref_ptr<glow::Buffer> m_indexBuffer;

    // Shadowing
    glow::ref_ptr<glow::Program> m_depthMapProgram;
    glow::ref_ptr<glow::Program> m_depthMapLinearizedProgram;
    glow::ref_ptr<glow::Program> m_shadowMappingProgram;
    virtual void initDepthMapProgram() = 0;
    virtual void initShadowMappingProgram() = 0;
};
