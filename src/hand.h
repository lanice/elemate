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

class Hand 
{
public:

    Hand();
    ~Hand();

    void draw(const glowutils::Camera & camera);

    glm::mat4 transform() const;
    glm::mat4 defaultTransform() const;

    glm::vec3 position() const;
    void setPosition(const glm::vec3 & position);

protected:
    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;
    glow::ref_ptr<glow::Buffer> m_indexBuffer;
    glow::ref_ptr<glow::Buffer> m_normalBuffer;

    glow::ref_ptr<glow::Program> m_program;

    unsigned int m_numVertices;

    glm::vec3 m_position;
    glm::mat4 m_translate;
    glm::mat4 m_transform;

    glm::mat4 m_defaultTransform;
};
