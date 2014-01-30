#include "userinterface.h"

#include "glow/Array.h"
#include "glow/Buffer.h"
#include "glow/VertexArrayObject.h"
#include "glow/VertexAttributeBinding.h"
#include "glow/Program.h"
#include "glow/Texture.h"

#include "glowutils/File.h"

#include "imagereader.h"

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

    m_program->setUniform("element_texture", 0);

    loadInitTexture("bedrock");
    loadInitTexture("sand");
}

void UserInterface::showHUD()
{
    m_stringDrawer.paint("Active element: wot�r", glm::mat4(0.5, 0, 0, 0, 
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
    drawPreviewCircle(0.94f, -0.95f, "bedrock");
    drawPreviewCircle(0.83f, -0.95f, "sand");
    drawPreviewCircle(0.72f, -0.95f, "sand");
    drawPreviewCircle(0.61f, -0.95f, "bedrock");
}

void UserInterface::drawPreviewCircle(float x, float y, const std::string& element)
{
    const float WIDTH = 0.10f; //Abh�nging von Breite --> resize

    m_textures.at(element)->bind(GL_TEXTURE0);

    m_program->setUniform("x", x);
    m_program->setUniform("y", y);
    m_program->setUniform("width", WIDTH);
    m_program->setUniform("ratio", m_viewport.x/m_viewport.y);

    m_program->use();
    m_vao->bind();

    m_vao->drawArrays(GL_TRIANGLE_STRIP,0,4);

    m_vao->unbind();
    m_program->release();

    m_textures.at(element)->unbind(GL_TEXTURE0);
}

void UserInterface::resize(int width, int height)
{
    m_viewport = glm::vec2(width, height);
    m_stringDrawer.resize(width, height);
}

void UserInterface::loadInitTexture(const std::string & elementName)
{
    glow::ref_ptr<glow::Texture> texture = new glow::Texture(GL_TEXTURE_2D);
    texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    RawImage image("data/textures/" + elementName + ".raw", 1024, 1024);

    texture->bind();
    texture->image2D(0, GL_RGB8, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, image.rawData());
    texture->unbind();

    m_textures.emplace(elementName, texture);
}