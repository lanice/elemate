#pragma once

#include <memory>
#include <vector>
#include <functional>

#include <glow/ref_ptr.h>

namespace glow {
    class FrameBufferObject;
    class Texture;
    class RenderBufferObject;
    class Program;
}
namespace glowutils {
    class ScreenAlignedQuad;
}
class World;
class RenderingStep;
class ParticleWaterStep;
class ShadowMappingStep;
class CameraEx;
class UserInterface;

class Renderer
{
public:
    Renderer(const World & world);

    void operator()(const CameraEx & camera);

    void resize(int width, int height);

    const glow::FrameBufferObject * sceneFbo() const;

    void addSceneFboReader(const std::function<void()> & reader);

protected:
    // drawing steps
    void sceneStep(const CameraEx & camera);
    void handStep(const CameraEx & camera);
    void userInterfaceStep(UserInterface * ui);
    std::shared_ptr<ParticleWaterStep> m_particleWaterStep;
    std::shared_ptr<ShadowMappingStep> m_shadowMappingStep;
    void flushStep(const CameraEx & camera);

    /** maintain a list of rendering all steps to apply operations on all of them, regardless of the ordering */
    std::vector<RenderingStep*> m_steps;

    const World & m_world;

    void initialize();

    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;
    glow::ref_ptr<glow::Program> m_quadProgram;

    glow::ref_ptr<glow::FrameBufferObject> m_sceneFbo;
    glow::ref_ptr<glow::Texture> m_sceneColor;
    glow::ref_ptr<glow::Texture> m_sceneDepth;

    glow::ref_ptr<glow::FrameBufferObject> m_handFbo;
    glow::ref_ptr<glow::Texture> m_sceneWithHandDepth;

    std::vector<std::function<void()>> m_sceneFboReader;
public:
    Renderer() = delete;
    Renderer(Renderer&) = delete;
    void operator=(Renderer&) = delete;
};
