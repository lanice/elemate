#include "manipulator.h"

#include <glow/logging.h>
#include <glow/FrameBufferObject.h>
#include "cameraex.h"

#include <glm/glm.hpp>
#include <glm/gtc/swizzle.hpp>

#include "world.h"
#include "navigation.h"
#include "hand.h"
#include "rendering/renderer.h"
#include "terrain/terraininteractor.h"
#include "particlescriptaccess.h"
#include "particlegroup.h"


Manipulator::Manipulator(GLFWwindow & window, const Navigation & navigation, World & world) :
m_window(window),
m_navigation(navigation),
m_camera(navigation.camera()),
m_world(world),
m_hand(*world.hand),
m_terrainInteractor(std::make_shared<TerrainInteractor>(m_world.terrain, "bedrock")),
m_grabbedTerrain(false),
m_renderer(nullptr)
{
    ParticleScriptAccess::instance().createParticleGroup("water");
}

Manipulator::~Manipulator()
{
}
void Manipulator::handleMouseButtonEvent(int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        ParticleScriptAccess::instance().particleGroup(0)->emit(100, m_hand.position(), glm::vec3(0,1,0));

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        ParticleScriptAccess::instance().particleGroup(0)->stopEmit();
}

void Manipulator::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & /*mods*/)
{
    const glm::vec3 & handPosition = m_hand.position();

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
            m_terrainInteractor->gatherElement(handPosition.x, handPosition.z, 0.1f);
            m_terrainInteractor->heightGrab(handPosition.x, handPosition.z);
            break;
        case GLFW_KEY_LEFT_ALT:
            m_grabbedTerrain = true;
            m_terrainInteractor->heightGrab(handPosition.x, handPosition.z);
            break;
        case GLFW_KEY_R:
            m_terrainInteractor->dropElement(handPosition.x, handPosition.z, 0.1f);
            m_terrainInteractor->heightGrab(handPosition.x, handPosition.z);
            break;
        case GLFW_KEY_C:
            const std::string & material = m_terrainInteractor->elementAt(handPosition.x, handPosition.z);
            glow::info("material at hand: ""%;""", material);
            break;
        }
    }

    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
        m_grabbedTerrain = false;
    }
}

void Manipulator::handleMouseMoveEvent(double xpos, double ypos)
{
    if (glfwGetKey(&m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        float delta = static_cast<float>(m_lastCursorPos.y - ypos);
        m_hand.setHeightOffset(m_hand.heightOffset() + 0.01f * delta);
        glfwSetCursorPos(&m_window, m_lastCursorPos.x, m_lastCursorPos.y);
    } else
        m_lastCursorPos = glm::dvec2(xpos, ypos);
}

void Manipulator::handleScrollEvent(const double & /*xoffset*/, const double & yoffset)
{
    if (glfwGetKey(&m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
    {
        if (yoffset > 0)
        {
            m_terrainInteractor->changeHeight(m_hand.position().x, m_hand.position().z, 0.1f);
            m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z);
        }
        else {
            m_terrainInteractor->changeHeight(m_hand.position().x, m_hand.position().z, -0.1f);
            m_terrainInteractor->heightGrab(m_hand.position().x, m_hand.position().z);
        }
    }
}

void Manipulator::updateHandPosition()
{
    glm::vec3 handPosition = objAt(glm::ivec2(glm::floor(m_lastCursorPos)));

    m_hand.setPosition(handPosition.x, handPosition.z);
    m_hand.rotate(m_navigation.rotationAngle());

    if (m_grabbedTerrain)
        m_terrainInteractor->heightPull(handPosition.x, handPosition.z);
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
