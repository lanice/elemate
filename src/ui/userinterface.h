#pragma once

#include "rendering/string_rendering/StringDrawer.h"
#include "glow/ref_ptr.h"
#include "glm/glm.hpp"

#include <unordered_map>

namespace glow{
    class VertexArrayObject;
    class Program;
}
struct GLFWwindow;

class UserInterface{
public:
    UserInterface(GLFWwindow& window);
    ~UserInterface();

    void initialize();
    void draw();
    void toggleHUD();
    void toggleMainMenu();

    bool isMainMenuOnTop() const;
    bool hasActiveHUD() const;

    void handleKeyEvent(int key, int scancode, int action, int mods);
    void handleScrollEvent(double xoffset, double yoffset);
    void handleMouseMoveEvent(double xpos, double ypos);
    void handleMouseButtonEvent(int button, int action, int mods);

    void resize(int width, int height);
protected:
    static const float kDefaultPreviewHeight;
    static const glm::vec3 kDefaultMenuEntryColor;
    static const glm::vec3 kDefaultHighlightedMenuEntryColor;

    bool m_mainMenuOnTop;
    bool m_activeHUD;

    StringDrawer m_stringDrawer;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Program>           m_previewProgram;
    glow::ref_ptr<glow::Program>           m_screenProgram;
    glm::vec2                              m_viewport;
    std::unordered_map<std::string, glow::ref_ptr<glow::Texture>>   m_textures;

    GLFWwindow& m_window;

    std::vector<std::string> m_menuEntries;
    unsigned int             m_activeMenuEntry;

    void drawHUD();
    void drawMainMenu();

    void drawGreyScreen();
    void drawMenuEntries();

    void drawPreview();
    void drawPreviewCircle(float x, float y, const std::string& element, float height);

    void loadInitTexture(const std::string & elementName);

    void invokeMenuEntryFunction();

private:
    UserInterface(const UserInterface&) = delete;
    void operator=(const UserInterface&) = delete;
};