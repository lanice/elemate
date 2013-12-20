#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


// class Camera;

class World;

class EventHandler
{
public:
    EventHandler(GLFWwindow & window, World & world);
    virtual ~EventHandler();

    void handleKeyEvent(int key, int scancode, int action, int mods);


protected:
    GLFWwindow & m_window;
    World & m_world;

    void operator=(EventHandler&) = delete;
};
