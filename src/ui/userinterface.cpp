#include "userinterface.h"

#include "glow/Array.h"
#include "glow/Buffer.h"
#include "glow/VertexArrayObject.h"
#include "glow/VertexAttributeBinding.h"
#include "glow/Program.h"
#include "glow/Texture.h"

#include "glowutils/File.h"

#include "GLFW/glfw3.h"

#include "imagereader.h"

const float UserInterface::kDefaultPreviewHeight = 0.1f;
const glm::vec3 UserInterface::kDefaultMenuEntryColor = glm::vec3(1.0f,1.0f,1.0f);
const glm::vec3 UserInterface::kDefaultHighlightedMenuEntryColor = glm::vec3(0.8f, 0.8f, 0.3f);

UserInterface::UserInterface(GLFWwindow& window) :
  m_activeHUD(true)
, m_mainMenuOnTop(false)
, m_window(window)
, m_activeMenuEntry(0)
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

    m_previewProgram = new glow::Program();
    m_previewProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/ui/preview.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/ui/preview.frag"));

    m_previewProgram->setUniform("element_texture", 0);

    m_screenProgram = new glow::Program();
    m_screenProgram->attach(
        glowutils::createShaderFromFile(GL_VERTEX_SHADER, "shader/ui/screen.vert"),
        glowutils::createShaderFromFile(GL_FRAGMENT_SHADER, "shader/ui/screen.frag"));

    loadInitTexture("bedrock");
    loadInitTexture("sand");
    loadInitTexture("water");
    loadInitTexture("lava");

    m_menuEntries.reserve(2);
    m_menuEntries.push_back("Fortsetzen");
    m_menuEntries.push_back("Beenden");
}

void UserInterface::draw()
{
    drawHUD();
    drawMainMenu();
}

void UserInterface::drawHUD()
{
    if (!m_activeHUD)
        return;

    drawPreview();
}

void UserInterface::drawMainMenu()
{
    if (!m_mainMenuOnTop)
        return;

    drawGreyScreen();
    drawMenuEntries();
}

void UserInterface::drawPreview()
{
    drawPreviewCircle(1.0f - 1 * kDefaultPreviewHeight, -0.97f + kDefaultPreviewHeight, "water", kDefaultPreviewHeight);
    drawPreviewCircle(1.0f - 3 * kDefaultPreviewHeight, -0.97f + kDefaultPreviewHeight, "lava", kDefaultPreviewHeight);
    drawPreviewCircle(1.0f - 5 * kDefaultPreviewHeight, -0.97f + kDefaultPreviewHeight, "sand", kDefaultPreviewHeight);
    drawPreviewCircle(1.0f - 7 * kDefaultPreviewHeight, -0.97f + kDefaultPreviewHeight, "bedrock", kDefaultPreviewHeight);
}

void UserInterface::drawPreviewCircle(float x, float y, const std::string& element, float height)
{
    m_textures.at(element)->bind(GL_TEXTURE0);

    m_previewProgram->setUniform("x", x);
    m_previewProgram->setUniform("y", y);
    m_previewProgram->setUniform("width", height);
    m_previewProgram->setUniform("ratio", m_viewport.x / m_viewport.y);

    m_previewProgram->use();
    m_vao->bind();

    m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao->unbind();
    m_previewProgram->release();

    m_textures.at(element)->unbind(GL_TEXTURE0);
}

void UserInterface::drawGreyScreen()
{
    glEnable(GL_BLEND);
    m_screenProgram->use();
    m_vao->bind();

    m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao->unbind();
    m_screenProgram->release();
    glDisable(GL_BLEND);
}

void UserInterface::drawMenuEntries()
{
    glm::vec3 color;
    for (unsigned int i = 0; i < m_menuEntries.size(); i++){
        if (i == m_activeMenuEntry)
            color = kDefaultHighlightedMenuEntryColor;
        else
            color = kDefaultMenuEntryColor;
        m_stringDrawer.paint(m_menuEntries[i],
            glm::mat4(1, 0, 0, 0,
                      0, 1, 0, 0,
                      0, 0, 1, 0,
                      0, 0.5-(i*1.0/m_menuEntries.size()), 0, 1),
                      StringDrawer::Alignment::kAlignCenter,
                      color);
    }    
}

void UserInterface::resize(int width, int height)
{
    m_viewport = glm::vec2(width, height);
    m_stringDrawer.resize(width, height);
}

void UserInterface::loadInitTexture(const std::string & elementName)
{
    const int TEXTURE_SIZE = 256;

    glow::ref_ptr<glow::Texture> texture = new glow::Texture(GL_TEXTURE_2D);
    texture->setParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture->setParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    texture->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    RawImage image("data/textures/preview/" + elementName + "_preview.raw", TEXTURE_SIZE, TEXTURE_SIZE);

    texture->bind();
    texture->image2D(0, GL_RGB8, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, image.rawData());
    texture->unbind();

    m_textures.emplace(elementName, texture);
}


void UserInterface::toggleHUD()
{
    m_activeHUD = !m_activeHUD;
}

void UserInterface::toggleMainMenu()
{
    m_mainMenuOnTop = !m_mainMenuOnTop;
}

bool UserInterface::isMainMenuOnTop() const 
{
    return m_mainMenuOnTop;
}

bool UserInterface::hasActiveHUD() const
{
    return m_activeHUD;
}

void UserInterface::invokeMenuEntryFunction()
{
    switch (m_activeMenuEntry)
    {
        case 0:     // Resume
            toggleMainMenu();
            break;
        case 1:     // Exit
            glfwSetWindowShouldClose(&m_window, GL_TRUE);
            break;
        default:
            break;
    }
}

void UserInterface::handleKeyEvent(int key, int /*scancode*/, int action, int /*mods*/)
{
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_W:    // Fallthrough
            case GLFW_KEY_UP:
                m_activeMenuEntry = m_activeMenuEntry > 0 ? m_activeMenuEntry - 1 : m_menuEntries.size() - 1; 
                break;
            case GLFW_KEY_S:
            case GLFW_KEY_DOWN: // Fallthrough
                m_activeMenuEntry = (m_activeMenuEntry + 1) % m_menuEntries.size();
                break;
            case GLFW_KEY_F:    // Fallthrough
            case GLFW_KEY_E:    // Fallthrough
            case GLFW_KEY_ENTER:
                invokeMenuEntryFunction();
                break;
            default:
                break;
        }
    }
}

void UserInterface::handleScrollEvent(double /*xoffset*/, double /*yoffset*/)
{

}

void UserInterface::handleMouseMoveEvent(double /*xpos*/, double /*ypos*/)
{

}

void UserInterface::handleMouseButtonEvent(int /*button*/, int /*action*/, int /*mods*/)
{

}