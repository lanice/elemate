#pragma once

#include "rendering/drawable.h"

#include <glowutils/CachedValue.h>

#include <glm/glm.hpp>

namespace glow {
    class VertexArrayObject;
    class Buffer;
}
class World;
class TerrainInteractor;

class Hand : public Drawable
{
public:
    Hand(const World & world);

    const glm::mat4 & transform() const;

    const glm::vec3 & position() const;
    /** set the hand position to the specified value */
    void setPosition(const glm::vec3 & position);
    /** set the hand position to the specified x, z coodinates and get the height from the terrain interactor 
      * Uses the hand's bounding box to ensure that it is above the terrain */
    void setPositionChecked(float worldX, float worldZ, const TerrainInteractor & interactor);

    float heightCheck(float worldX, float worldZ, const TerrainInteractor & interactor);

    void rotate(float angle);

protected:
    void loadModel();
    static const std::string s_modelFilename;

    virtual void drawImplementation(const glowutils::Camera & camera) override;
    virtual void drawLightMapImpl(const CameraEx & lightSource) override;
    virtual void drawShadowMappingImpl(const glowutils::Camera & camera, const CameraEx & lightSource) override;

    glow::ref_ptr<glow::Buffer> m_normalBuffer;

    glow::ref_ptr<glow::Program> m_program;

    int m_numIndices;

    glow::Vec3Array m_heightCheckPoints;

    glm::vec3 m_position;

    glm::mat4 m_translate;
    glm::mat4 m_rotate;

    glowutils::CachedValue<glm::mat4> m_transform;

    virtual void initLightMappingProgram() override;
    virtual void initShadowMappingProgram() override;

public:
    void operator=(Hand&) = delete;
};
