#include "game.h"

#include <thread>
#include <cassert>

#include <glow/logging.h>
#include "cameraex.h"

#include <GLFW/glfw3.h>

// Own Classes
#include "physicswrapper.h"
#include "world.h"
#include "particledrawable.h"
#include "terrain/terrain.h"
#include "rendering/renderer.h"


Game::Game(GLFWwindow & window) :
m_window(window),
m_vsyncEnabled(true),
m_physicsWrapper(new PhysicsWrapper),
m_world(new World(*m_physicsWrapper)),
m_camera(std::make_shared<CameraEx>(ProjectionType::perspective)),
m_navigation(window, m_camera, m_world->terrain),
m_manipulator(window, m_navigation, *m_world),
m_renderer(*m_world)
{
    setVSync(m_vsyncEnabled);

    m_world->setNavigation(m_navigation);
    m_manipulator.setRenderer(m_renderer);

    m_userInterface.initialize();
}

Game::~Game()
{
    delete m_world;
    delete m_physicsWrapper;
}

void Game::showMenu()
{
    m_world->togglePause();

    m_userInterface.showMainMenu();
}

void Game::start()
{
    glfwSetTime(0.0);   

    m_world->togglePause();

    loop();
}

void Game::loop(double delta)
{
    double nextTime = glfwGetTime();
    double maxTimeDiff = 0.5;
    int skippedFrames = 1;
    int maxSkippedFrames = 5;

    double drawTime = nextTime;

    while (!glfwWindowShouldClose(&m_window))
    {
        glfwPollEvents();
        // get current time
        double currTime = glfwGetTime();

        // are we too far far behind? then do drawing step now.
        if ((currTime - nextTime) > maxTimeDiff)
            nextTime = currTime;

        if (currTime >= nextTime)
        {
            nextTime += delta;

            m_world->updatePhysics();

            // update and draw objects if we have time remaining or already too many frames skipped.
            if ((currTime < nextTime) || (skippedFrames > maxSkippedFrames))
            {
                double deltaTime = glfwGetTime() - drawTime;
                drawTime = glfwGetTime();
                
                m_navigation.update(deltaTime);
                m_navigation.apply();

                m_world->updateVisuals();
                
                m_renderer(*m_camera);
                m_userInterface.showHUD();

                glfwSwapBuffers(&m_window);

                skippedFrames = 1;
            } else {
                ++skippedFrames;
            }
        // Don't need to sleep because vsync is on.
        // } else {
        //     double sleepTime = nextTime - currTime;

        //     if (sleepTime > 0)
        //         std::this_thread::sleep_for(std::chrono::milliseconds(int(sleepTime * 1000)));
        }
    }

    m_world->stopSimulation();
}

void Game::setVSync(bool enabled)
{
    m_vsyncEnabled = enabled;
    if (m_vsyncEnabled)
        glow::debug("Enabling VSync");
    else
        glow::debug("Disabling VSync");

    glfwSwapInterval(m_vsyncEnabled ? 1 : 0);
}

void Game::toggleVSync()
{
    setVSync(!m_vsyncEnabled);
}

bool Game::vSyncEnabled() const
{
    return m_vsyncEnabled;
}

Navigation * Game::navigation()
{
    return & m_navigation;
}

Manipulator * Game::manipulator()
{
    return & m_manipulator;
}

std::shared_ptr<CameraEx> Game::camera()
{
    return m_camera;
}

const std::shared_ptr<const CameraEx> Game::camera() const
{
    return m_camera;
}

Renderer * Game::renderer()
{
    return &m_renderer;
}

PhysicsWrapper * Game::physicsWrapper()
{
    return m_physicsWrapper;
}
