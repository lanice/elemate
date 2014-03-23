#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


class Game;

/** @brief Receives all incoming input events. Delegates to Manipulator/Navigation/UserInterface if necesarry. */
class EventHandler
{
public:
    EventHandler(GLFWwindow & window, Game & game);
    virtual ~EventHandler();

    /** Delegates inputs to UserInterface while menu screen is on top, to Manipulator otherwise. */
    void handleMouseButtonEvent(int button, int action, int mods);
    /** Delegates inputs to UserInterface while menu screen is on top. Handles Metagame-related keys like debug-keys or shader reloading. Delegates rest to Manipulator/Navigation. */
    void handleKeyEvent(int key, int scancode, int action, int mods);
    /** Delegates inputs to UserInterface while menu screen is on top, to Manipulator/Navigation otherwise. */
    void handleScrollEvent(double xoffset, double yoffset);
    /** Delegates inputs to UserInterface while menu screen is on top, to Manipulator otherwise. */
    void handleMouseMoveEvent(double xpos, double ypos);
    /** Adjusts window width/heigth. */
    void handeResizeEvent(int width, int height);


protected:
    GLFWwindow & m_window;
    Game & m_game;

public:
    EventHandler() = delete;
    void operator=(EventHandler&) = delete;
};
