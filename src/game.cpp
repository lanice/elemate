#include "game.h"

#include <thread>
#include <cassert>

#include <glow/VertexArrayObject.h>
#include <glow/VertexAttributeBinding.h>
#include <glow/Buffer.h>
#include <glow/Array.h>
#include <glow/Program.h>

#include <GLFW/glfw3.h>

// Own Classes
#include "world.h"
#include "particledrawable.h"


Game::Game(GLFWwindow & window) :
m_window(window),
m_world(std::make_shared<World>()),
m_camera(),
m_navigation(window, &m_camera),
m_manipulator(window, *m_world),
m_paused(false)
{
}

Game::~Game()
{}

void Game::start()
{
    glfwSetTime(0.0);

    m_world->setNavigation(m_navigation);    

    m_world->startSimulation();

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

            if (!m_paused)
                m_world->update();

            // update and draw objects if we have time remaining or already too many frames skipped.
            if ((currTime < nextTime) || (skippedFrames > maxSkippedFrames))
            {
                double deltaTime = glfwGetTime() - drawTime;
                drawTime = glfwGetTime();
                
                m_navigation.update(deltaTime);
                m_navigation.apply();

                m_manipulator.updateHandPosition(*m_navigation.camera());

                draw();

                glfwSwapBuffers(&m_window);

                skippedFrames = 1;
            } else {
                ++skippedFrames;
            }
        } else {
            double sleepTime = nextTime - currTime;

            if (sleepTime > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(int(sleepTime * 1000)));
        }
    }

    m_world->stopSimulation();
}

void Game::togglePause()
{
    m_paused = !m_paused;
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


static glow::VertexArrayObject * m_vao = nullptr;
static glow::Buffer * m_vertices = nullptr;

void initDraw()
{
    m_vao = new glow::VertexArrayObject;
    m_vao->bind();

    m_vertices = new glow::Buffer(GL_ARRAY_BUFFER);

    static const glow::Vec3Array raw(
    {
          glm::vec3(-1.f, 0.0f, -1.f)
        , glm::vec3(+1.f, 0.0f, -1.f)
        , glm::vec3(+1.f, 0.0f, +1.f)
        , glm::vec3(-1.f, 0.0f, +1.f)
    });
    
    m_vertices->setData(raw, GL_STATIC_DRAW);

    glow::VertexAttributeBinding * vertexBinding = m_vao->binding(0);
    vertexBinding->setAttribute(0); // location 0
    vertexBinding->setBuffer(m_vertices, 0, sizeof(glm::vec3)); // stride must be size of datatype!
    vertexBinding->setFormat(3, GL_FLOAT, GL_FALSE, 0);
    m_vao->enable(0);

    m_vao->unbind();
}

void Game::draw()
{
    if (!m_vao)
        initDraw();

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glow::ref_ptr<glow::Program> terrainProgram = m_world->programByName("terrainPlain");
    assert(terrainProgram);
    if (!terrainProgram)
        return;

    terrainProgram->use();

    m_world->setUniforms(*terrainProgram.get());

    m_vao->bind();
    m_vao->drawArrays(GL_QUADS, 0, 4);
    m_vao->unbind();

    terrainProgram->release();

    ParticleDrawable::drawParticles(*m_navigation.camera());

}
