#pragma once

#include "rendering/string_rendering/StringDrawer.h"
#include "glow/ref_ptr.h"
#include "glm/glm.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <list>

namespace glow{
    class VertexArrayObject;
    class Program;
}

class MenuPage;
struct GLFWwindow;
class LuaWrapper;

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

    void registerLuaFunctions(LuaWrapper * lua);
    
protected:
    static const float kDefaultPreviewHeight;
    static const glm::vec3 kDefaultMenuEntryColor;
    static const glm::vec3 kDefaultHighlightedMenuEntryColor;

    bool m_mainMenuOnTop;
    bool m_activeHUD;

    std::list<TextObject> m_hudTexts;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Program>           m_previewProgram;
    glow::ref_ptr<glow::Program>           m_screenProgram;
    glm::vec2                              m_viewport;
    std::unordered_map<std::string, glow::ref_ptr<glow::Texture>>   m_textures;

    GLFWwindow& m_window;

    std::unordered_map<std::string, MenuPage*> m_menus;
    std::string                                m_activeMenu;

    std::vector<std::string> m_elementPreviews;
    unsigned int             m_activeElement;
    
	void drawHUD();
	void drawMainMenu();

    void drawGreyScreen();
    void drawMenuEntries();
    void drawAchievements();

    void drawPreview();
    void drawPreviewCircle(float x, float y, const std::string& element, float height, bool highlighted);

    void loadInitTexture(const std::string & elementName);

    void invokeMenuEntryFunction();

private:
    UserInterface(const UserInterface&) = delete;
    void operator=(const UserInterface&) = delete;
};