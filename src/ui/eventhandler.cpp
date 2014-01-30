#include "eventhandler.h"

#include <glow/logging.h>
#include <glowutils/FileRegistry.h>
#include "cameraex.h"

#include <GLFW/glfw3.h>

#include "game.h"
#include "rendering/renderer.h"
#include "physicswrapper.h"
#include "lua/luawrapper.h"

EventHandler::EventHandler(GLFWwindow & window, Game & game)
: m_window(window)
, m_game(game)
{
     glfwSetInputMode(&window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

EventHandler::~EventHandler()
{
}


void EventHandler::handleMouseButtonEvent(int button, int action, int mods)
{
    if (m_game.userInterface()->isMainMenuOnTop())
    {
        m_game.userInterface()->handleMouseButtonEvent(button, action, mods);
        return;
    }
    
    m_game.manipulator()->handleMouseButtonEvent(button, action, mods);
}
    
void EventHandler::handleKeyEvent(int key, int scancode, int action, int mods)
{
    if (m_game.userInterface()->isMainMenuOnTop())
    {
        m_game.userInterface()->handleKeyEvent(key, scancode, action, mods);
        return;
    }
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            m_game.toggleMenu();
            break;
        case GLFW_KEY_F5:
            glow::info("Updating shader...");
            glowutils::FileRegistry::instance().reloadAll();
            glow::info("Updating shader done.");
            glow::info("Reloading lua scripts...");
            LuaWrapper::reloadAll();
            glow::info("Reloading lua scripts done.");
            break;
        case GLFW_KEY_V:
            m_game.toggleVSync();
            break;
        case GLFW_KEY_G:
            m_game.physicsWrapper()->toogleUseGpuParticles();
            break;
        }
    }

    m_game.navigation()->handleKeyEvent(key, scancode, action, mods);
    m_game.manipulator()->handleKeyEvent(key, scancode, action, mods);
}

void EventHandler::handleMouseMoveEvent(double xpos, double ypos)
{
    if (m_game.userInterface()->isMainMenuOnTop())
    {
        m_game.userInterface()->handleMouseMoveEvent(xpos, ypos);
        return;
    }
    m_game.manipulator()->handleMouseMoveEvent(xpos, ypos);
}

void EventHandler::handleScrollEvent(double xoffset, double yoffset)
{
    if (m_game.userInterface()->isMainMenuOnTop())
    {
        m_game.userInterface()->handleScrollEvent(xoffset, yoffset);
        return;
    }

    m_game.navigation()->handleScrollEvent(xoffset, yoffset);
    m_game.manipulator()->handleScrollEvent(xoffset, yoffset);
}

void EventHandler::handeResizeEvent(int width, int height)
{
    glViewport(0, 0, width, height);
    m_game.resize(width, height);
}
