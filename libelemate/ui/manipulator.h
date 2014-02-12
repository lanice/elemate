#pragma once

#include <memory>

#include <glow/global.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>


class World;
class Navigation;
class Hand;
class TerrainInteractor;
class LuaWrapper;
class CameraEx;


class Manipulator
{
public:
    Manipulator(GLFWwindow & window, const Navigation & navigation, World & world);
    virtual ~Manipulator();

    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);
    void handleMouseButtonEvent(int button, int action, int mods);
    void handleMouseMoveEvent(double xpos, double ypos);
    void handleScrollEvent(const double & xoffset, const double & yoffset);

    void updateHandPosition();

    LuaWrapper * lua();

protected:
    void registerLuaFunctions(LuaWrapper * lua);

    void setGrabbedTerrain(bool grabbed);

    GLFWwindow & m_window;
    const Navigation & m_navigation;
    const CameraEx & m_camera;
    World & m_world;
    Hand & m_hand;
    std::shared_ptr<TerrainInteractor> m_terrainInteractor;
    bool m_grabbedTerrain;
    LuaWrapper * m_lua;

    glm::dvec2 m_lastCursorPos;

public:
    Manipulator() = delete;
    void operator= (Manipulator&) = delete;
};
