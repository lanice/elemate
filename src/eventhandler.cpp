#include "eventhandler.h"


#include <glow/logging.h>


EventHandler::EventHandler(GLFWwindow & window/*, Camera * camera*/) :
    m_window(window)/*,
    m_camera(camera)*/
{
}


EventHandler::~EventHandler()
{
}


void EventHandler::handleKeyEvent(int key, int scancode, int action, int mods)
{
    glow::debug("Receiving KeyEvent: key %;, scancode %;, action %;, modifiers %;", key, scancode, action, mods);
}
