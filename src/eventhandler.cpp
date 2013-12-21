#include "eventhandler.h"

#include <glow/logging.h>
#include <glowutils/FileRegistry.h>

#include <GLFW/glfw3.h>

#include "game.h"


EventHandler::EventHandler(GLFWwindow & window, Game & game) :
    m_window(window),
    m_game(game)
{
}

EventHandler::~EventHandler()
{
}


void EventHandler::handleKeyEvent(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        m_game.end();
    }
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        glow::info("Updating shader...");
        glowutils::FileRegistry::instance().reloadAll();
        glow::info("Updating shader done.");
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
