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
    void showHUD();
    void showMainMenu();

    void resize(int width, int height);
protected:
    static const float kDefaultPreviewHeight;

    StringDrawer m_stringDrawer;

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Program>           m_program;
    glm::vec2                              m_viewport;
    std::unordered_map<std::string, glow::ref_ptr<glow::Texture>>   m_textures;

    void drawPreview();
    void drawPreviewCircle(float x, float y, const std::string& element, float height);

    void loadInitTexture(const std::string & elementName);

private:
    UserInterface(const UserInterface&) = delete;
    void operator=(const UserInterface&) = delete;
};