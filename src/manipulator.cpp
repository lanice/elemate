#include "manipulator.h"


#include "world.h"


Manipulator::Manipulator(GLFWwindow & window, World & world) :
    m_window(window),
    m_world(world)
{
}

Manipulator::~Manipulator()
{
}


void Manipulator::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & /*mods*/)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
        m_world.makeStandardBall(glm::vec3(0, 1, 0));
    if (key == GLFW_KEY_I && action == GLFW_PRESS)
        m_world.toggleBackgroundSound(0);
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        m_world.toggleBackgroundSound(1);
}
