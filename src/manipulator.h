#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


namespace glowutils { class Camera; }

class World;

class Manipulator
{
public:
    Manipulator(GLFWwindow & window, World & world);
    virtual ~Manipulator();

    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);

    void updateHandPosition(const glowutils::Camera & camera);

    void updateListener(const glowutils::Camera & camera);


protected:
    GLFWwindow & m_window;
    World & m_world;

    void operator= (Manipulator&) = delete;
};
