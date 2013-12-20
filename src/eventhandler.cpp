#include "eventhandler.h"

#include <glow/logging.h>

#include <GLFW/glfw3.h>

#include "world.h"


EventHandler::EventHandler(GLFWwindow & window, World & world) :
    m_window(window),
    m_world(world)
{
}

EventHandler::~EventHandler()
{
}


void EventHandler::handleKeyEvent(int key, int scancode, int action, int mods)
{
    glow::debug("Receiving KeyEvent: key %;, scancode %;, action %;, modifiers %;", key, scancode, action, mods);
    if (key == GLFW_KEY_F) {
        m_world.makeStandardBall(glm::vec3(0, 1, 0));
    }
}
