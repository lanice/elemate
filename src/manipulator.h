#pragma once

#include <memory>

#include <glow/global.h>

#include <GLFW/glfw3.h>


namespace glowutils { class Camera; }

class World;
class TerrainInteractor;

class Manipulator
{
public:
    Manipulator(GLFWwindow & window, World & world);
    virtual ~Manipulator();

    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);

    void updateHandPosition(const glowutils::Camera & camera);


protected:
    GLFWwindow & m_window;
    World & m_world;
    std::shared_ptr<TerrainInteractor> m_terrainInteractor;
    bool m_grabbedTerrain;

    void operator= (Manipulator&) = delete;
};
