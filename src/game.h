#pragma once

#include <memory>

#include <glow/global.h>

#include "ui/navigation.h"
#include "ui/manipulator.h"
#include "rendering/renderer.h"
#include "ui/userinterface.h"

class PhysicsWrapper;
class World;
namespace std {         class thread; }
struct GLFWwindow;
class CameraEx;

/** The Game Class that invokes a game loop and initializes PhysX.
 *  To receive the initialized physics, call getPhysicsWrapper(). See for its usage the documentation of PhysicsWrapper class.
 *  Typical Workflow is instantiating the Game class, call start() and let it run until the game ends, then call end().
 */
class Game{
public:
    Game(GLFWwindow & window);

    ~Game();

    /** Starts the Game Loop until recieving a window close event. */
    void start();
    void toggleMenu();

    void setVSync(bool enabled);
    void toggleVSync();
    bool vSyncEnabled() const;
    void resize(int width, int height);

    Navigation * navigation();
    Manipulator * manipulator();
    UserInterface * userInterface();

    std::shared_ptr<CameraEx> camera();
    const std::shared_ptr<const CameraEx> camera() const;
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