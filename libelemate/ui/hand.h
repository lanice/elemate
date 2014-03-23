#pragma once

#include "rendering/shadowingdrawable.h"

#include <string>
#include <vector>

#include <glowutils/CachedValue.h>

#include <glm/glm.hpp>

class LuaWrapper;

/** @brief Representation of the ingame Hand. */
class Hand : public ShadowingDrawable
{
public:
    Hand();

    virtual const glm::mat4 & transform() const override;

    const glm::vec3 & position() const;
    /** Sets the hand position to the specified x, z coordinates and get the height from the m_world's terrain.
      * Uses the hand's bounding box to ensure that it is above the terrain. */
    void setPosition(float worldX, float worldZ);
    /** Sets the y-distance between the terrain and the hand. */
    void setHeightOffset(float heightOffset);
    float heightOffset() const;

    /** Rotates the hand model by given angle. */
    void rotate(float angle);

    /** Registers functions of the Hand that can be used within given LuaWrapper instance. */
    void registerLuaFunctions(LuaWrapper * lua);

protected:
    void loadModel();
    static const std::string s_modelFilename;

    virtual void drawImplementation(const CameraEx & camera) override;
    virtual void drawDepthMapImpl(const CameraEx & camera) override;
    virtual void drawShadowMappingImpl(const CameraEx & camera, const CameraEx & lightSource) override;

    glow::ref_ptr<glow::Buffer> m_normalBuffer;

    glow::ref_ptr<glow::Program> m_program;

    int m_numIndices;

    std::vector<glm::vec3> m_heightCheckPoints;
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

    virtual void initDepthMapProgram() override;
    virtual void initShadowMappingProgram() override;

public:
    void operator=(Hand&) = delete;
};
