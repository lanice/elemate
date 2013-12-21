#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


class World;

class Manipulator
{
public:
    Manipulator(GLFWwindow & window, World & world);
    virtual ~Manipulator();

    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);


protected:
    GLFWwindow & m_window;
    World & m_world;

    void operator= (Manipulator&) = delete;
};
