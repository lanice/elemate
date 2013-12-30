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

    glm::mat4 transform();
    glm::mat4 defaultTransform();

    glm::vec3 position();

protected:
    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;
    glow::ref_ptr<glow::Buffer> m_indexBuffer;
    glow::ref_ptr<glow::Buffer> m_normalBuffer;

    glow::ref_ptr<glow::Program> m_program;

    unsigned int m_numVertices;


    glm::mat4 m_transform;

    glm::mat4 _defaultTransform;
};
