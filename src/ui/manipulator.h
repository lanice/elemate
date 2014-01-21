#pragma once

#include <memory>

#include <glow/global.h>
#include <glowutils/AbstractCoordinateProvider.h>

#include <GLFW/glfw3.h>


namespace glowutils { class Camera; }

class World;
class Navigation;
class Hand;
class TerrainInteractor;
class Renderer;

class Manipulator : public glowutils::AbstractCoordinateProvider
{
public:
    Manipulator(GLFWwindow & window, const Navigation & navigation, World & world);
    virtual ~Manipulator();

    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);
    void handleMouseButtonEvent(int button, int action, int mods);

    void updateHandPosition();

    /** This is testing code and should probably go somewhere else */
    void convertTerrainAtHand();

    void setRenderer(Renderer & renderer);

    virtual const float depthAt(const glm::ivec2 & windowCoordinates) override;

    using AbstractCoordinateProvider::objAt;

    virtual const glm::vec3 objAt(const glm::ivec2 & windowCoordinates, const float depth) override;

    virtual const glm::vec3 objAt(const glm::ivec2 & windowCoordinates
        , const float depth
        , const glm::mat4 & viewProjectionInverted) override;

protected:
    GLFWwindow & m_window;
    const Navigation & m_navigation;
    const glowutils::Camera & m_camera;
    World & m_world;
    Hand & m_hand;
    std::shared_ptr<TerrainInteractor> m_terrainInteractor;
    bool m_grabbedTerrain;
    Renderer * m_renderer;

public:
    Manipulator() = delete;
    void operator= (Manipulator&) = delete;
};
