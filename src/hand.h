#pragma once

#include "rendering/drawable.h"

#include <glm/glm.hpp>

namespace glow {
    class VertexArrayObject;
    class Buffer;
}
class World;

class Hand : public Drawable
{
public:
    Hand(const World & world);
    ~Hand();

    glm::mat4 transform() const;

    glm::vec3 position() const;
    void setPosition(const glm::vec3 & position);

    void rotate(const float angle);

protected:
    virtual void drawImplementation(const glowutils::Camera & camera) override;
    virtual void drawLightMapImpl(const CameraEx & lightSource) override;
    virtual void drawShadowMappingImpl(const glowutils::Camera & camera, const CameraEx & lightSource) override;

    glow::ref_ptr<glow::Buffer> m_normalBuffer;

    glow::ref_ptr<glow::Program> m_program;

    int m_numVertices;
    int m_numIndices;

    glm::vec3 m_position;
    glm::mat4 m_translate;
    glm::mat4 m_rotate;
    glm::mat4 m_scale;
    glm::mat4 m_transform;

    virtual void initLightMappingProgram() override;
    virtual void initShadowMappingProgram() override;

public:
    void operator=(Hand&) = delete;
};
