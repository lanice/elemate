#pragma once

#include <memory>

#include <glowutils/Camera.h>

#include "ui/navigation.h"
#include "ui/manipulator.h"
#include "renderer.h"

class PhysicsWrapper;
class World;
namespace std {         class thread; }
struct GLFWwindow;

/** The Game Class that invokes a game loop and initializes PhysX.
 *  To receive the initialized physics, call getPhysicsWrapper(). See for its usage the documentation of PhysicsWrapper class.
 *  Typical Workflow is instantiating the Game class, call start() and let it run until the game ends, then call end().
 */
class Game{
public:
    Game(GLFWwindow & window);

    ~Game();

    /** Starts the Game Loop until end() is called.  */
    void start();


    void reloadLua();

    Navigation * navigation();
    Manipulator * manipulator();
    glowutils::Camera * camera();
    Renderer * renderer();


protected:
    GLFWwindow & m_window;

    /** The Game's loop containing drawing and triggering physics is placed right here.
      * @param delta specifies the time between each logic update in seconds.*/
    void loop(double delta = 1.0/100.0);

    PhysicsWrapper * m_physicsWrapper;
    World * m_world;

    glowutils::Camera m_camera;
    Navigation m_navigation;
    Manipulator m_manipulator;

    Renderer m_renderer;

public:
    Game() = delete;
    void operator=(Game &) = delete;
};