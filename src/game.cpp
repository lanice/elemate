#include "game.h"

#include <thread>
#include <cassert>

#include <glow/global.h>

#include <GLFW/glfw3.h>

// Own Classes
#include "physicswrapper.h"
#include "world.h"
#include "particledrawable.h"
#include "terrain/terrain.h"
#include "renderer.h"


Game::Game(GLFWwindow & window) :
m_window(window),
m_physicsWrapper(new PhysicsWrapper),
m_world(new World(*m_physicsWrapper)),
m_camera(),
m_navigation(window, &m_camera, m_world->terrain),
m_manipulator(window, *m_world),
m_renderer(*m_world)
{
    m_world->setNavigation(m_navigation);
}

Game::~Game()
{
    delete m_world;
    delete m_physicsWrapper;
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

            m_world->update();

            // update and draw objects if we have time remaining or already too many frames skipped.
            if ((currTime < nextTime) || (skippedFrames > maxSkippedFrames))
            {
                double deltaTime = glfwGetTime() - drawTime;
                drawTime = glfwGetTime();
                
                m_navigation.update(deltaTime);
                m_navigation.apply();

                m_manipulator.updateHandPosition(*m_navigation.camera());
                m_world->updateListener();

                m_renderer(m_camera);

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

Navigation * Game::navigation()
{
    return & m_navigation;
}

Manipulator * Game::manipulator()
{
    return & m_manipulator;
}

glowutils::Camera * Game::camera()
{
    return & m_camera;
}
