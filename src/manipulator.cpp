#include "manipulator.h"

#include <glowutils/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/swizzle.hpp>

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
    // As soon as we can access sth like m_world->terrain->heightAt(pos.x, pos.z) we will use that instead.
    pos.y = 0.5; // For now.

    // Final step, as soon as we have the Hand.
    // m_hand->transform()->setMatrix( m_hand->defaultTransform()/* * rotationMatrix*/ * glm::translate( pos ) );

    updateListener(camera);
}

void Manipulator::updateListener(const glowutils::Camera & camera){
    glm::vec3 forward = glm::normalize(camera.eye() - camera.center());

    m_world.updateListener(camera.eye(), forward, camera.up());
}
