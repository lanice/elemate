#include "manipulator.h"

#include <glow/logging.h>
#include <glow/FrameBufferObject.h>

#include <glm/glm.hpp>
#include <glm/gtc/swizzle.hpp>

#include "world.h"
#include "navigation.h"
#include "hand.h"
#include "rendering/renderer.h"
#include "terrain/terraininteractor.h"


Manipulator::Manipulator(GLFWwindow & window, const Navigation & navigation, World & world) :
m_window(window),
m_navigation(navigation),
m_camera(*navigation.camera()),
m_world(world),
m_hand(*world.hand),
m_terrainInteractor(std::make_shared<TerrainInteractor>(m_world.terrain)),
m_grabbedTerrain(false),
m_renderer(nullptr)
{
    m_world.makeElements(m_hand.position());
    m_world.createFountainSound(m_hand.position());
}

Manipulator::~Manipulator()
{
}
void Manipulator::handleMouseButtonEvent(int button, int action, int /*mods*/)
{
    //Emitter Interaction
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        m_world.startEmitting();
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        m_world.stopEmitting();
}

void Manipulator::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & mods)
{
    bool altPressed = (mods & GLFW_MOD_ALT) == GLFW_MOD_ALT;

    // key press events
    if (action == GLFW_PRESS)
    {
        switch (key) {
        case GLFW_KEY_I:
            m_world.toggleBackgroundSound(0);
            break;
        case GLFW_KEY_O:
            m_world.toggleBackgroundSound(0);
            break;
        case GLFW_KEY_P:
            m_world.togglePause();
            break;
        case GLFW_KEY_F:
            if (!altPressed)
                m_world.selectNextEmitter();
            else {
                m_terrainInteractor->changeHeight(m_hand.position().x, m_hand.position().z, TerrainLevel::BaseLevel, -0.1f);
                m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z, TerrainLevel::BaseLevel);
            }
            break;
        case GLFW_KEY_LEFT_ALT:
            m_grabbedTerrain = true;
            m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z, TerrainLevel::BaseLevel);
            break;
        case GLFW_KEY_R:
            if (altPressed) {
                m_terrainInteractor->changeHeight(m_hand.position().x, m_hand.position().z, TerrainLevel::BaseLevel, 0.1f);
                m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z, TerrainLevel::BaseLevel);
            }
            break;
        }
    }

    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
        m_grabbedTerrain = false;
    }
}

void Manipulator::updateHandPosition()
{
    double xpos, ypos;
    glfwGetCursorPos(&m_window, &xpos, &ypos);

    glm::vec3 handPosition = objAt(glm::ivec2(std::floor(xpos), std::floor(ypos)));

    if (m_grabbedTerrain)
        m_terrainInteractor->heightPull(handPosition.x, handPosition.z);

    m_hand.setPosition(handPosition);
    m_hand.rotate(m_navigation.rotationAngle());

    m_world.updateEmitterPosition(handPosition);
}

void Manipulator::setRenderer(Renderer & renderer)
{
    m_renderer = &renderer;
    // call updateHandPosition when the renderer has drawn the scene
    m_renderer->addSceneFboReader(std::bind(&Manipulator::updateHandPosition, this));
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