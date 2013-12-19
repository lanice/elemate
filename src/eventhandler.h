#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


// class Camera;

class EventHandler
{
public:
    EventHandler(GLFWwindow & window);
    virtual ~EventHandler();

    void handleKeyEvent(int key, int scancode, int action, int mods);


protected:
    GLFWwindow & m_window;
};
