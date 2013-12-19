#include "navigation.h"

#include <glow/logging.h>
// #include <glowutils/Camera.h>


Navigation::Navigation(GLFWwindow & window, glowutils::Camera * camera) :
    m_window(window),
    m_camera(camera)
{
    glow::debug("TODO: setup camera.");
}

Navigation::~Navigation()
{
}


void Navigation::update()
{
}
