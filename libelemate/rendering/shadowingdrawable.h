#pragma once

#include "drawable.h"

namespace glow {
    class Program;
}

class ShadowingDrawable : public Drawable
{
public:
    ShadowingDrawable();

    /** writes the linearized depth into the current depth attachment */
    virtual void drawDepthMap(const CameraEx & camera);
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
