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
m_terrainInteractor(std::make_shared<TerrainInteractor>(m_world.terrain, "bedrock")),
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

    if (key == GLFW_KEY_I && action == GLFW_PRESS)
        m_world.toggleBackgroundSound(0);
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        m_world.toggleBackgroundSound(1);
    
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        m_world.togglePause();

    // Emitter Interaction
    if (key == GLFW_KEY_F && action == GLFW_PRESS && !altPressed)
        m_world.selectNextEmitter();


    // Terrain interaction
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        m_grabbedTerrain = true;
        m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z);
    }
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
        m_grabbedTerrain = false;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS && altPressed) {
        m_terrainInteractor->changeHeight(m_hand.position().x, m_hand.position().z, 0.1f);
        m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z);
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS && altPressed) {
        m_terrainInteractor->changeHeight(m_hand.position().x, m_hand.position().z, -0.1f);
        m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z);
    }
}

void Manipulator::updateHandPosition()
{
    double xpos, ypos;
    glfwGetCursorPos(&m_window, &xpos, &ypos);

    glm::vec3 handPosition = objAt(glm::ivec2(std::floor(xpos), std::floor(ypos)));

    m_hand.setPosition(handPosition.x, handPosition.z);
    m_hand.rotate(m_navigation.rotationAngle());

    if (m_grabbedTerrain)
        m_terrainInteractor->heightPull(handPosition.x, handPosition.z);

    m_world.updateEmitterPosition(handPosition);
}

void Manipulator::setRenderer(Renderer & renderer)
{
    m_renderer = &renderer;
}

const float Manipulator::depthAt(const glm::ivec2 & windowCoordinates)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderer->sceneFbo()->id());
    const float depth =  AbstractCoordinateProvider::depthAt(m_camera, GL_DEPTH_COMPONENT, windowCoordinates);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return depth;
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