#include "eventhandler.h"

#include <glow/logging.h>


EventHandler::EventHandler(GLFWwindow & window) :
    m_window(window)
{
}

EventHandler::~EventHandler()
{
}


void EventHandler::handleKeyEvent(int key, int scancode, int action, int mods)
{
    glow::debug("Receiving KeyEvent: key %;, scancode %;, action %;, modifiers %;", key, scancode, action, mods);
}
