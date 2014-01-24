#include "eventhandler.h"

#include <glow/logging.h>
#include <glowutils/FileRegistry.h>
#include "cameraex.h"

#include <GLFW/glfw3.h>

#include "game.h"
#include "rendering/renderer.h"
#include "physicswrapper.h"


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
    m_game.manipulator()->handleMouseButtonEvent(button, action, mods);
}
    
void EventHandler::handleKeyEvent(int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(&m_window, GL_TRUE);
            break;
        case GLFW_KEY_F5:
            glow::info("Updating shader...");
            glowutils::FileRegistry::instance().reloadAll();
            glow::info("Updating shader done.");
            glow::info("Reloading lua scripts...");
            m_game.reloadLua();
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

void EventHandler::handleScrollEvent(double xoffset, double yoffset)
{
    // As soon as we have a HUD interface in which we scroll,
    // e.g. to choose an element, add a condition here,
    // as we don't want the navigation to scroll if we are "navigating" in our HUD.

    // if (HUD_active)
    // {
    //     m_game->hud()->handleScrollEvent(xoffset, yoffset);
    //     return;
    // }

    // or something like this...

    m_game.navigation()->handleScrollEvent(xoffset, yoffset);
}

void EventHandler::handeResizeEvent(int width, int height)
{
    glViewport(0, 0, width, height);
    m_game.camera()->setViewport(width, height);

    m_game.renderer()->resize(width, height);
}
