#pragma once

#include <memory>
#include <vector>
#include <map>

#include <glm/glm.hpp>

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
class RenderingStep;
class ParticleStep;
class ShadowMappingStep;
class CameraEx;
class UserInterface;
class DebugStep;

/** Encapsulated the rendering pipeline.

    Basic steps:
        - draw the solid terrain and the hand, get the color and depth image from both.
        - draw the particles see ParticleStep.
        - call the ShadowMappingStep to create light and shadow maps.
        - combine the results in the flush shader. */
class Renderer
{
public:
    Renderer();

    void render(const CameraEx & camera);

    void resize(int width, int height);

    const glow::FrameBufferObject * sceneFbo() const;

    bool drawDebugInfo() const;
    void toggleDrawDebugInfo();
    void setDrawDebugInfo(bool doDraw);
    void toggleDrawHeatMap();

    void takeScreenShot();
    void writeScreenShot();

protected:
    // drawing steps
    void sceneStep(const CameraEx & camera);
    void handStep(const CameraEx & camera);
    std::shared_ptr<DebugStep> m_debugStep;
    bool m_drawDebugStep;
    bool m_drawHeatMap;
    void userInterfaceStep(UserInterface * ui);
    std::shared_ptr<ParticleStep> m_particleStep;
    std::shared_ptr<ShadowMappingStep> m_shadowMappingStep;
    void flushStep(const CameraEx & camera);

    /** maintain a list of rendering all steps to apply operations on all of them, regardless of the ordering */
    std::vector<RenderingStep*> m_steps;

    bool m_takeScreenShot;

    glm::ivec2 m_viewport;

    void initialize();
    void initSkybox();
    glow::ref_ptr<glow::Texture> m_skyboxTexture;

    typedef std::pair<std::string, glow::ref_ptr<glow::Texture>> NamedTexture;
    typedef std::pair<std::string, glow::ref_ptr<glow::FrameBufferObject>> NamedFbo;

    /** list of textures to be used as flush shader input. The list index determines the sampler id. */
    std::vector<NamedTexture> m_flushSources;

    /** named textures created by the renderer */
    std::map<std::string, glow::ref_ptr<glow::Texture>> m_textureByName;
    std::map<std::string, glow::ref_ptr<glow::FrameBufferObject>> m_fboByName;

    glow::ref_ptr<glowutils::ScreenAlignedQuad> m_quad;
    glow::ref_ptr<glow::Program> m_quadProgram;
    glow::ref_ptr<glow::Texture> m_rainTexture;

public:
    Renderer(Renderer&) = delete;
    void operator=(Renderer&) = delete;
};
