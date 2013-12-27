#include "manipulator.h"

#include <glowutils/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/swizzle.hpp>

#include "world.h"
#include "terrain/terraininteractor.h"


Manipulator::Manipulator(GLFWwindow & window, World & world) :
m_window(window),
m_world(world),
m_terrainInteractor(std::make_shared<TerrainInteractor>(m_world.terrain))
{
}

Manipulator::~Manipulator()
{
}


void Manipulator::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & /*mods*/)
{
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        m_world.makeStandardBall(glm::vec3(0, 1, 0));
        m_world.createFountainSound(glm::vec3(0, 1, 0));
    }
    if (key == GLFW_KEY_I && action == GLFW_PRESS)
        m_world.toggleBackgroundSound(0);
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        m_world.toggleBackgroundSound(1);
    
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        m_world.togglePause();
}

void Manipulator::updateHandPosition(const glowutils::Camera & camera)
{
    double xpos, ypos;
    glfwGetCursorPos(&m_window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(&m_window, &width, &height);

    glm::vec3 eye = camera.eye();
    glm::vec3 center = camera.center();

    glm::mat4 viewProjectionInverted = camera.viewProjectionInverted();

    double x = (xpos / float(width) - 0.5) * 2.;
    double y = (ypos / float(height) - 0.5) * 2.;

    glm::vec4 position = viewProjectionInverted * glm::vec4(x, -y, 1., 1.);
    glm::vec3 lookAtWorld = glm::vec3(glm::swizzle<glm::X, glm::Y, glm::Z>(position)) / position.w;

    glm::vec3 pos = eye - (lookAtWorld * eye.y / lookAtWorld.y);

    pos.y = m_terrainInteractor->heightAt(pos.x, pos.z);

    // Final step, as soon as we have the Hand.
    // m_hand->transform()->setMatrix( m_hand->defaultTransform()/* * rotationMatrix*/ * glm::translate( pos ) );
}