#include "manipulator.h"

#include <glowutils/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/swizzle.hpp>

#include "world.h"
#include "terrain/terraininteractor.h"


Manipulator::Manipulator(GLFWwindow & window, World & world) :
m_window(window),
m_world(world),
m_terrainInteractor(std::make_shared<TerrainInteractor>(m_world.terrain)),
m_grabbedTerrain(false)
{
}

Manipulator::~Manipulator()
{
}

// until we have a hand
static glm::vec3 handPosition;

void Manipulator::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & mods)
{
    bool altPressed = (mods & GLFW_MOD_ALT) == GLFW_MOD_ALT;

    if (key == GLFW_KEY_F && action == GLFW_PRESS && !altPressed)
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


    // Terrain interaction
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        m_grabbedTerrain = true;
        m_terrainInteractor->heightGrab(handPosition.x, handPosition.z, TerrainLevel::BaseLevel);
    }
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
        m_grabbedTerrain = false;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS && altPressed) {
        m_terrainInteractor->changeHeight(handPosition.x, handPosition.z, TerrainLevel::BaseLevel, 0.1f);
        m_terrainInteractor->heightGrab(handPosition.x, handPosition.z, TerrainLevel::BaseLevel);
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS && altPressed) {
        m_terrainInteractor->changeHeight(handPosition.x, handPosition.z, TerrainLevel::BaseLevel, -0.1f);
        m_terrainInteractor->heightGrab(handPosition.x, handPosition.z, TerrainLevel::BaseLevel);
    }
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

    // until than:
    handPosition = pos;

    if (m_grabbedTerrain)
        m_terrainInteractor->heightPull(pos.x, pos.z);
}