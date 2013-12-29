#include "manipulator.h"

#include <glow/logging.h>
#include <glowutils/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/swizzle.hpp>

#include "world.h"
#include "terrain/terraininteractor.h"


Manipulator::Manipulator(GLFWwindow & window, const glowutils::Camera & camera, World & world) :
m_window(window),
m_camera(camera),
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

void Manipulator::updateHandPosition()
{
    double xpos, ypos;
    glfwGetCursorPos(&m_window, &xpos, &ypos);

    handPosition = objAt(glm::ivec2(std::floor(xpos), std::floor(ypos))) + glm::vec3(0.0f, 0.5f, 0.0f);

    if (m_grabbedTerrain)
        m_terrainInteractor->heightPull(handPosition.x, handPosition.z);

    // Final step, as soon as we have the Hand.
    // m_hand->transform()->setMatrix( m_hand->defaultTransform()/* * rotationMatrix*/ * glm::translate( pos ) );
}

const float Manipulator::depthAt(const glm::ivec2 & windowCoordinates)
{
    return AbstractCoordinateProvider::depthAt(m_camera, GL_DEPTH_COMPONENT, windowCoordinates);
}

const glm::vec3 Manipulator::objAt(const glm::ivec2 & windowCoordinates, const float depth)
{
    return unproject(m_camera, depth, windowCoordinates);
}

const glm::vec3 Manipulator::objAt(
    const glm::ivec2 & windowCoordinates
    , const float depth
    , const glm::mat4 & viewProjectionInverted)
{
    return unproject(m_camera, viewProjectionInverted, depth, windowCoordinates);
}