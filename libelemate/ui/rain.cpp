#include "rain.h"
#include <vector>
#include <glow/Buffer.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Program.h>
#include <glowutils/global.h>
#include "utils/cameraex.h"

Rain::Rain()
{
}

Rain::~Rain()
{
}

void Rain::initialize()
{
    std::vector<glm::vec3> points({
        glm::vec3(0.5f, 0.5f, -0.5f)
    });

    glow::Buffer* vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    vbo->setData(points, GL_STATIC_DRAW);

    m_vao = new glow::VertexArrayObject();

    m_vao->bind();

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);

    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(vbo, 0, sizeof(glm::vec3));
    vertexBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_vao->unbind();

    m_rainProgram = new glow::Program();
    m_rainProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/ui/rain.vert"),
        glowutils::createShaderFromFile(GL_GEOMETRY_SHADER, "shader/ui/rain.geo"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/ui/rain.frag")
    );
}

void Rain::draw(const CameraEx & camera)
{
    /*m_rainProgram->setUniform("x", 0);
    m_rainProgram->setUniform("y", 0);
    m_rainProgram->setUniform("width", height);
    m_rainProgram->setUniform("ratio", m_viewport.x / m_viewport.y);
    m_rainProgram->setUniform("highlighted", static_cast<int>(highlighted));
    m_rainProgram->setUniform("element_texture", TextureManager::getTextureUnit("UserInterface", element));*/
    m_rainProgram->setUniform("projection", camera.projectionEx());

    m_rainProgram->use();
    m_vao->bind();

    m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao->unbind();
    m_rainProgram->release();
}

void Rain::resize(int width, int height)
{
    m_viewport = glm::vec2(width, height);
}