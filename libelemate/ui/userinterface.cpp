#include "userinterface.h"

#include <glow/Buffer.h>
#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Program.h>
#include <glow/Texture.h>
#include <glow/logging.h>

#include <glowutils/global.h>

#include <GLFW/glfw3.h>

#include "lua/luawrapper.h"
#include "io/imagereader.h"
#include "menupage.h"

const float UserInterface::kDefaultPreviewHeight = 0.1f;
const glm::vec3 UserInterface::kDefaultMenuEntryColor = glm::vec3(1.0f,1.0f,1.0f);
const glm::vec3 UserInterface::kDefaultHighlightedMenuEntryColor = glm::vec3(0.8f, 0.8f, 0.3f);

UserInterface::UserInterface(GLFWwindow& window) :
  m_activeHUD(true)
, m_mainMenuOnTop(false)
, m_window(window)
, m_activeElement(1000) //Definitely out of range --> no initial highlighting
{
}

UserInterface::~UserInterface()
{
}

void UserInterface::initialize()
{
    m_stringDrawer.initialize();
    std::vector<glm::vec2> points({
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

    m_elementPreviews.clear();
    m_elementPreviews.reserve(4);

    loadInitTexture("bedrock");
    loadInitTexture("lava");
    loadInitTexture("sand");
    loadInitTexture("water");

    m_activeMenu = "MainMenu";
    m_menus.reserve(3);
    m_menus.emplace("MainMenu", new MenuPage("MainMenu"));
    m_menus["MainMenu"]->addEntry("Fortsetzen");
    m_menus["MainMenu"]->addEntry("Hilfe");
    m_menus["MainMenu"]->addEntry("Beenden");
    m_menus.emplace("Settings", new MenuPage("Settings"));
    m_menus["Settings"]->addEntry("Zurück zur Hauptseite");
    m_menus["Settings"]->addEntry("GPU Berechnung umschalten");
    m_menus["Settings"]->addEntry("VSync umschalten ");
    m_menus.emplace("Help", new MenuPage("Help"));
    m_menus["Help"]->addEntry("Zurück zur Hauptseite");
    m_menus["Help"]->addEntry("");
    m_menus["Help"]->addEntry("Linksklicken zum Freisetzen des aktiven Elements");
    m_menus["Help"]->addEntry("Scrollen zum Wählen des aktiven Elements");
    m_menus["Help"]->addEntry("Rechtsklicken zum Sammeln eines Elements");
    m_menus["Help"]->addEntry("Strg halten und scrollen zum Kippen der Kamera");
    m_menus["Help"]->addEntry("Shift halten und scrollen zum Zoomen");
    m_menus["Help"]->addEntry("Esc nimmt das Spiel wieder auf");
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
    for (const auto& text : m_hudTexts){
        m_stringDrawer.paint(text);
    }
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
    for (unsigned int i = 0; i < m_elementPreviews.size(); i++){
        drawPreviewCircle(1.0f - (1+2*i) * kDefaultPreviewHeight, -0.97f + kDefaultPreviewHeight, m_elementPreviews[i], kDefaultPreviewHeight, m_activeElement==i);
    }
}

void UserInterface::drawPreviewCircle(float x, float y, const std::string& element, float height, bool highlighted)
{
    m_textures.at(element)->bindActive(GL_TEXTURE0);

    m_previewProgram->setUniform("x", x);
    m_previewProgram->setUniform("y", y);
    m_previewProgram->setUniform("width", height);
    m_previewProgram->setUniform("ratio", m_viewport.x / m_viewport.y);
    m_previewProgram->setUniform("highlighted", static_cast<int>(highlighted));

    m_previewProgram->use();
    m_vao->bind();

    m_vao->drawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao->unbind();
    m_previewProgram->release();

    m_textures.at(element)->unbindActive(GL_TEXTURE0);
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
    float distance = 1.0f / m_menus[m_activeMenu]->entryCount();
    for (unsigned int i = 0; i < m_menus[m_activeMenu]->entryCount(); i++){
        if (i == m_menus[m_activeMenu]->activeEntry())
            color = kDefaultHighlightedMenuEntryColor;
        else
            color = kDefaultMenuEntryColor;
        m_stringDrawer.paint(m_menus[m_activeMenu]->entryCaption(i),
            glm::mat4(0.5, 0, 0, 0,
                      0, 0.5, 0, 0,
                      0, 0, 0.5, 0,
                      0, 0.5 - (i*distance), 0, 1),
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
    m_elementPreviews.push_back(elementName);

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
    m_activeMenu = "MainMenu";
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
    if (m_activeMenu == "MainMenu")
        switch (m_menus[m_activeMenu]->activeEntry())
        {
            case 0:     // Resume
                toggleMainMenu();
                break;
            case 1:     // Help
                m_activeMenu = "Help";
                break;
            case 2:     // Settings
                /* Settings have no effect, therefore disabled
                m_activeMenu = "Settings";
                break;
            case 3:     //Exit 
                */
                glfwSetWindowShouldClose(&m_window, GL_TRUE);
                break;
            default:
                toggleMainMenu();
                break;
        }
    else if (m_activeMenu == "Help")
        switch (m_menus[m_activeMenu]->activeEntry())
        {
            case 0:     // Back to Main Menu
                m_activeMenu = "MainMenu";
                break;
            default:
                break;
        }
    else if (m_activeMenu == "Settings")
        switch (m_menus[m_activeMenu]->activeEntry())
        {
            case 0:     // Back to Main Menu
                m_activeMenu = "MainMenu";
                break;
            case 1:     // Toggle GPU Calculation
                // Toggle GPU calculation
                break;
            case 2:     // Toggle VSync
                // Toggle VSync
                break;
            default:
                break;
        }
    else
        toggleMainMenu();
}

void UserInterface::handleKeyEvent(int key, int /*scancode*/, int action, int /*mods*/)
{
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_W:    // Fallthrough
            case GLFW_KEY_UP:
                m_menus[m_activeMenu]->highlightPreviousEntry();
                break;
            case GLFW_KEY_S:
            case GLFW_KEY_DOWN: // Fallthrough
                m_menus[m_activeMenu]->highlightNextEntry();
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

void UserInterface::handleScrollEvent(double /*xoffset*/, double yoffset)
{
    if (yoffset > 0)
        m_menus[m_activeMenu]->highlightPreviousEntry();
    else 
        m_menus[m_activeMenu]->highlightNextEntry();
}

void UserInterface::handleMouseMoveEvent(double /*xpos*/, double /*ypos*/)
{
    //No representation of our cursor, therefore no reaction
}

void UserInterface::handleMouseButtonEvent(int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && isMainMenuOnTop())
        invokeMenuEntryFunction();
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && isMainMenuOnTop())
        m_activeMenu = "MainMenu";
}

void UserInterface::registerLuaFunctions(LuaWrapper * lua)
{
    std::function<unsigned int()> func0 = [=] ()
    { return m_activeElement; };

    std::function<unsigned int(unsigned int)> func1 = [=] (unsigned int index)
    { m_activeElement = index; return 0;};

    std::function<unsigned int()> clearTexts = [=]()
    { m_hudTexts.clear(); return 0; };

    std::function<unsigned int(std::string)> debugText = [=](std::string debug_message)
    {   
        m_hudTexts.clear();
        TextObject t; t.text = debug_message;
        t.x = -1.0f; t.y = -0.95f; t.z = 0.0f; t.scale = 0.5f;
        t.red = 0.5f; t.blue = t.green = 0.0f;
        m_hudTexts.push_back(t);
        return 0; 
    };

    std::function<unsigned int(std::string, float, float, float, float, float, float, float)> writeText = [=]
        (std::string debug_message, float x, float y, float z, float scale, float red, float green, float blue)
    {   
        TextObject t; t.text = debug_message; 
        t.x = x; t.y = y; t.z = z; t.scale = scale;
        t.red = red; t.blue = blue; t.green = green;
        m_hudTexts.push_back(t);
        return 0; 
    };

    lua->Register("hud_activeElement", func0);
    lua->Register("hud_setActiveElement", func1);
    lua->Register("hud_addText", writeText);
    lua->Register("hud_clearTexts", clearTexts);
    lua->Register("hud_debugText", debugText);
}
