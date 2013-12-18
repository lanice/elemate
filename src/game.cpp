#include "game.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

#include <GLFW/glfw3.h>

// Own Classes
#include "world.h"
#include "physicswrapper.h"
#include "objectscontainer.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"


Game::Game(GLFWwindow & window) :
m_window(window),
m_interrupted(true),
m_world(std::make_shared<World>()),
m_cyclicTime(new CyclicTime(0.0L, 1.0L))
{
}

Game::~Game()
{}

void Game::start(){
    if (isRunning())
        return;

    m_world->reloadShader();
    

    m_world->physics_wrapper->startSimulation();

    loop();
}

void Game::loop(t_longf delta){
    m_interrupted = false;

    t_longf nextTime = m_cyclicTime->getNonModf(true);
    t_longf maxTimeDiff = 0.5L;
    int skippedFrames = 1;
    int maxSkippedFrames = 5;

    while (isRunning())
    {
        // get current time
        t_longf currTime = m_cyclicTime->getNonModf(true);

        // are we too far far behind? then do drawing step now.
        if ((currTime - nextTime) > maxTimeDiff)
            nextTime = currTime;

        if (currTime >= nextTime)
        {
            nextTime += delta;

            // update physic
            if (m_world->physics_wrapper->step())
                // physx: each simulate() call must be followed by fetchResults()
                m_world->objects_container->updateAllObjects();

            // update and draw objects if we have time remaining or already too many frames skipped.
            if ((currTime < nextTime) || (skippedFrames > maxSkippedFrames))
            {
                m_world->setUniforms(currTime);

                glfwSwapBuffers(&m_window);

                glfwPollEvents();

                skippedFrames = 1;
            } else {
                ++skippedFrames;
            }
        } else {
            //t_longf sleepTime = nextTime - currTime;

            /*if (sleepTime > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(int(sleepTime * 1000)));*/
        }
    }

    m_interrupted = true;
    m_world->physics_wrapper->stopSimulation();
}

bool Game::isRunning()const{
    return !(/*m_window. || */m_interrupted);
}

void Game::end(){
    if (isRunning())
        m_interrupted = true;
}
