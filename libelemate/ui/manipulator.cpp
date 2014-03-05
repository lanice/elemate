#include "manipulator.h"

#include <glow/logging.h>
#include <glow/FrameBufferObject.h>
#include "utils/cameraex.h"

#include <glm/glm.hpp>
#include <glm/gtc/swizzle.hpp>

#include "world.h"
#include "navigation.h"
#include "hand.h"
#include "terrain/terraininteractor.h"
#include "particles/particlescriptaccess.h"
#include "particles/particlegroup.h"
#include "lua/luawrapper.h"
#include "ui/achievementmanager.h"

Manipulator::Manipulator(GLFWwindow & window, const Navigation & navigation, World & world) :
m_window(window),
m_navigation(navigation),
m_camera(navigation.camera()),
m_world(world),
m_hand(*world.hand),
m_terrainInteractor(std::make_shared<TerrainInteractor>(m_world.terrain, "bedrock")),
m_grabbedTerrain(false),
m_lua(new LuaWrapper())
{
    registerLuaFunctions(m_lua);
    AchievementManager::instance()->registerLuaFunctions(m_lua);
    
    ParticleScriptAccess::instance().registerLuaFunctions(m_lua);
    m_hand.registerLuaFunctions(m_lua);
    m_world.registerLuaFunctions(m_lua);
    m_terrainInteractor->registerLuaFunctions(m_lua);

    m_lua->loadScript("scripts/manipulator.lua");
}

Manipulator::~Manipulator()
{
    delete m_lua;
    ParticleScriptAccess::release();
}
void Manipulator::handleMouseButtonEvent(int button, int action, int /*mods*/)
{
    m_lua->call("handleMouseButtonEvent", button, action);
}

void Manipulator::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & /*mods*/)
{
    m_lua->call("handleKeyEvent", key, action);
    const glm::vec3 & handPosition = m_hand.position();

    // key press events
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        glow::info("material at hand: ""%;"", solid element: ""%;""", m_terrainInteractor->topmostElementAt(handPosition.x, handPosition.z), m_terrainInteractor->solidElementAt(handPosition.x, handPosition.z));
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
    else if (glfwGetKey(&m_window, GLFW_KEY_X) == GLFW_PRESS)
    {
        m_lua->call("handleScrollEvent", yoffset);
    }
}

void Manipulator::updateHandPosition()
{
    int windowWidth, windowHeight;
    double cursorX, cursorY;
    glfwGetWindowSize(&m_window, &windowWidth, &windowHeight);
    glfwGetCursorPos(&m_window, &cursorX, &cursorY);

    float normX = static_cast<float>(cursorX) / windowWidth  * 2.f - 1.0f;
    float normY = 1.0f - static_cast<float>(cursorY) / windowHeight * 2.f;

    glm::vec4 viewpos = m_camera.viewProjectionInvertedEx() * glm::vec4(normX, normY, -1.0f, 1.0f);
    viewpos /= viewpos.w;    
    glm::vec3 viewDir = glm::vec3(viewpos) - m_camera.eye();

    float navigationHeight = m_navigation.center().y;

    glm::vec3 handPosition = m_camera.eye() + ((navigationHeight - m_camera.eye().y) / viewDir.y) * viewDir;

    m_hand.setPosition(handPosition.x, handPosition.z);
    m_hand.rotate(m_navigation.rotationAngle());

    if (m_grabbedTerrain)
        m_terrainInteractor->heightPull(handPosition.x, handPosition.z);

    m_lua->call("updateHandPosition", handPosition.x, m_hand.position().y, handPosition.z);
}

LuaWrapper * Manipulator::lua()
{
    return m_lua;
}

void Manipulator::registerLuaFunctions(LuaWrapper * lua)
{
    std::function<int(bool)> func0 = [=] (bool grabbed)
    { setGrabbedTerrain(grabbed); return 0; };

    std::function<int(int)> func1 = [=] (int key)
    { return glfwGetKey(&m_window, key); };

    lua->Register("manipulator_setGrabbedTerrain", func0);
    lua->Register("glfw_getKey", func1);
}

void Manipulator::setGrabbedTerrain(bool grabbed)
{
    m_grabbedTerrain = grabbed;
}
