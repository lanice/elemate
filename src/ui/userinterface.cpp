#include "userinterface.h"

#include "glow/Array.h"
#include "glow/Buffer.h"
#include "glow/VertexArrayObject.h"
#include "glow/VertexAttributeBinding.h"
#include "glow/Program.h"
#include "glowutils/File.h"

UserInterface::UserInterface()
{
}

UserInterface::~UserInterface()
{
}

void UserInterface::initialize()
{
    m_stringDrawer.initialize();
    glow::Vec2Array points({
        glm::vec2(+1.f, -1.f)
        , glm::vec2(+1.f, +1.f)
        , glm::vec2(-1.f, -1.f)
        , glm::vec2(-1.f, +1.f)
    });

    glow::Buffer* vbo = new glow::Buffer(GL_ARRAY_BUFFER);
    vbo->setData(points, GL_STATIC_DRAW);

    m_vao = new glow::VertexArrayObject();

    m_vao->bind();

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);

    vertexBinding->setAttribute(0);
    vertexBinding->setBuffer(vbo, 0, sizeof(glm::vec2));
    vertexBinding->setFormat(2, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_vao->unbind();

    m_program = new glow::Program();
    m_program->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER,   "shader/preview.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/preview.frag"));
}

void UserInterface::showHUD()
{
    m_stringDrawer.paint("Active element: wotör", glm::mat4(0.5, 0, 0, 0, 
                                                            0, 0.5, 0, 0, 
                                                            0, 0, 0.5, 0, 
                                                            -0.95, -0.95, 0, 1), StringDrawer::Alignment::kAlignLeft);
    drawPreview();
}

void UserInterface::showMainMenu()
{
}

void UserInterface::drawPreview()
{
    drawPreviewCircle(0.94f, -0.95f);
    drawPreviewCircle(0.83f, -0.95f);
    drawPreviewCircle(0.72f, -0.95f);
    drawPreviewCircle(0.61f, -0.95f);
}

void UserInterface::drawPreviewCircle(float x, float y)
{
    const float WIDTH = 0.05; //Abhänging von Breite --> resize

    m_program->setUniform("x", x);
    m_program->setUniform("y", y);
    m_program->setUniform("width", WIDTH);
    m_program->setUniform("ratio", m_viewport.x/m_viewport.y);

    m_program->use();
    m_vao->bind();

    m_vao->drawArrays(GL_TRIANGLE_STRIP,0,4);

    m_vao->unbind();
    m_program->release();
}

void UserInterface::resize(int width, int height)
{
    m_viewport = glm::vec2(width, height);
}