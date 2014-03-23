#pragma once

#include <memory>

#include <glow/global.h>

#include "ui/navigation.h"
#include "ui/manipulator.h"
#include "rendering/renderer.h"
#include "ui/userinterface.h"

class PhysicsWrapper;
class World;
struct GLFWwindow;
class CameraEx;

/** @brief Initializes the physics and game contents and maintains the game loop. */
class Game
{
public:
    Game(GLFWwindow & window);
    ~Game();

    /** Starts the Game Loop until receiving a window close event. */
    void start();

    /** enable or disable vertical synchronization */
    void setVSync(bool enabled);
    void toggleVSync();
    bool vSyncEnabled() const;
    void resize(int width, int height);

    Navigation * navigation();
    Manipulator * manipulator();
    UserInterface * userInterface();

    Renderer * renderer();
    PhysicsWrapper * physicsWrapper();

protected:
    GLFWwindow & m_window;

    /** The Game's loop containing drawing and triggering physics is placed right here.
      * @param delta specifies the time between each logic update in seconds.*/
    void loop(double delta = 1.0 / 100.0);

    bool m_vsyncEnabled;

    PhysicsWrapper * m_physicsWrapper;
    World * m_world;

    std::shared_ptr<CameraEx> m_camera;
    Navigation m_navigation;
    Manipulator m_manipulator;

    Renderer m_renderer;
    UserInterface m_userInterface;

public:
    Game() = delete;
    void operator=(Game &) = delete;
};