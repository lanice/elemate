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
    /** set the hand position to the specified x, z coodinates and get the height from the m_world's terrain 
      * Uses the hand's bounding box to ensure that it is above the terrain */
    void setPosition(float worldX, float worldZ);
    /** set the y-distance between the terrain and the hand */
    void setHeightOffset(float heightOffset);
    float heightOffset() const;

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
    float m_heightOffset;
    /** get the terrain height at all height checkpoints and calculate a transition value between them, + m_heightOffset*/
    float heightCheck(float worldX, float worldZ) const;

    float m_positionX;
    /** height depending on terrain height at height checkpoints + m_heightOffset */
    glowutils::CachedValue<float> m_positionY;
    float m_positionZ;
    glowutils::CachedValue<glm::vec3> m_position;

    const glm::mat4 & translate() const;
    glowutils::CachedValue<glm::mat4> m_translate;
    glm::mat4 m_rotate;

    glowutils::CachedValue<glm::mat4> m_transform;
    const glm::mat3 & xzTransform() const;
    glowutils::CachedValue<glm::mat3> m_xzTransform; // this is needed to transform the heightCheckpoints while calculating the actual transform

    virtual void initLightMappingProgram() override;
    virtual void initShadowMappingProgram() override;

public:
    void operator=(Hand&) = delete;
};
