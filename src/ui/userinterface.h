#pragma once

#include "rendering/string_rendering/StringDrawer.h"
#include "glow/ref_ptr.h"
#include "glm/glm.hpp"

#include <unordered_map>

namespace glow{
    class VertexArrayObject;
    class Program;
}

class UserInterface{
public:
    UserInterface();
    ~UserInterface();

    void initialize();
    void draw();
    void toggleHUD();
    void toggleMainMenu();

    bool isMainMenuOnTop() const;

    void resize(int width, int height);
protected:
    static const float kDefaultPreviewHeight;

    bool m_mainMenuOnTop;
    bool m_visibleHUD;

    StringDrawer m_stringDrawer;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Program>           m_previewProgram;
    glow::ref_ptr<glow::Program>           m_screenProgram;
    glm::vec2                              m_viewport;
    std::unordered_map<std::string, glow::ref_ptr<glow::Texture>>   m_textures;

    void drawHUD();
    void drawMainMenu();

    void drawGreyScreen();

    void drawPreview();
    void drawPreviewCircle(float x, float y, const std::string& element, float height);

    void loadInitTexture(const std::string & elementName);

private:
    UserInterface(const UserInterface&) = delete;
    void operator=(const UserInterface&) = delete;
};