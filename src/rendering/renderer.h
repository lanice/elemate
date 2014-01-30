#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <map>

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
    std::shared_ptr<ParticleWaterStep> m_particleWaterStep;
    std::shared_ptr<ShadowMappingStep> m_shadowMappingStep;
    void flushStep(const CameraEx & camera);

    /** maintain a list of rendering all steps to apply operations on all of them, regardless of the ordering */
    std::vector<RenderingStep*> m_steps;

    const World & m_world;

    float timef;

    void initialize();

    typedef std::pair<std::string, glow::ref_ptr<glow::Texture>> NamedTexture;
    typedef std::pair<std::string, glow::ref_ptr<glow::FrameBufferObject>> NamedFbo;

    /** list of textures to be used as flush shader input. The list index determines the sampler id. */
    std::vector<NamedTexture> m_flushSources;

    /** named textures created by the renderer */
    std::map<std::string, glow::ref_ptr<glow::Texture>> m_textureByName;
    std::map<std::string, glow::ref_ptr<glow::FrameBufferObject>> m_fboByName;

    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;
    glow::ref_ptr<glow::Program> m_quadProgram;

    std::vector<std::function<void()>> m_sceneFboReader;
public:
    Renderer() = delete;
    Renderer(Renderer&) = delete;
    void operator=(Renderer&) = delete;
};
