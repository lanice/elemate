#pragma once

#include <glow/ref_ptr.h>

#include <glm/glm.hpp>

namespace glow {
    class VertexArrayObject;
    class Buffer;
    class Program;
}
namespace glowutils {
    class Camera;
}
class World;
class CameraEx;

class Hand 
{
public:
    Hand(const World & world);
    ~Hand();

    void draw(const glowutils::Camera & camera);
    /** writes the linearized depth into the current depth attachment */
    virtual void drawLightMap(const CameraEx & lightSource);
    virtual void drawShadowMapping(const glowutils::Camera & camera, const CameraEx & lightSource);

    glm::mat4 transform() const;

    glm::vec3 position() const;
    void setPosition(const glm::vec3 & position);

    void rotate(const float angle);

protected:
    const World & m_world;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;
    glow::ref_ptr<glow::Buffer> m_indexBuffer;
    glow::ref_ptr<glow::Buffer> m_normalBuffer;

    glow::ref_ptr<glow::Program> m_program;

    int m_numVertices;
    int m_numIndices;

    glm::vec3 m_position;
    glm::mat4 m_translate;
    glm::mat4 m_rotate;
    glm::mat4 m_scale;
    glm::mat4 m_transform;

    // Shadowing
    glow::ref_ptr<glow::Program> m_lightMapProgram;
    void initShadowMappingProgram();
    glow::ref_ptr<glow::Program> m_shadowMappingProgram;

public:
    void operator=(Hand&) = delete;
};
