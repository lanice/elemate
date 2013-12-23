#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


class Game;

class EventHandler
{
public:
    EventHandler(GLFWwindow & window, Game & game);
    virtual ~EventHandler();

    void handleKeyEvent(int key, int scancode, int action, int mods);
    void handleScrollEvent(double xoffset, double yoffset);


protected:
    GLFWwindow & m_window;
    Game & m_game;

    void operator=(EventHandler&) = delete;
};
