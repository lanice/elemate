#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


class Game;

class EventHandler
{
public:
    EventHandler(GLFWwindow & window, Game & game);
    virtual ~EventHandler();

    void handleMouseButtonEvent(int button, int action, int mods);
    void handleKeyEvent(int key, int scancode, int action, int mods);
    void handleScrollEvent(double xoffset, double yoffset);
    void handleMouseMoveEvent(double xpos, double ypos);
    void handeResizeEvent(int width, int height);


protected:
    GLFWwindow & m_window;
    Game & m_game;

public:
    EventHandler() = delete;
    void operator=(EventHandler&) = delete;
};
