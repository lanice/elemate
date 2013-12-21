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
#include "physicswrapper.h"
#include "objectscontainer.h"
#include "particledrawable.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"


Game::Game(GLFWwindow & window) :
m_window(window),
m_interrupted(true),
m_world(std::make_shared<World>()),
m_camera(),
m_navigation(window, &m_camera),
m_cyclicTime(new CyclicTime(0.0L, 1.0L))
{
}

Game::~Game()
{}

void Game::start(){
    if (isRunning())
        return;

    m_world->setNavigation(m_navigation);    

    m_world->physicsWrapper->startSimulation();

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
        glfwPollEvents();
        // get current time
        t_longf currTime = m_cyclicTime->getNonModf(true);

        // are we too far far behind? then do drawing step now.
        if ((currTime - nextTime) > maxTimeDiff)
            nextTime = currTime;

        if (currTime >= nextTime)
        {
            nextTime += delta;

            // update physic
            if (m_world->physicsWrapper->step())
                // physx: each simulate() call must be followed by fetchResults()
                m_world->objectsContainer->updateAllObjects();

            // update and draw objects if we have time remaining or already too many frames skipped.
            if ((currTime < nextTime) || (skippedFrames > maxSkippedFrames))
            {
                m_navigation.update();
                m_navigation.apply();

                draw();

                glfwSwapBuffers(&m_window);

                skippedFrames = 1;
            } else {
                ++skippedFrames;
            }
        } else {
            t_longf sleepTime = nextTime - currTime;

            if (sleepTime > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(int(sleepTime * 1000)));
        }
    }

    m_interrupted = true;
    m_world->physicsWrapper->stopSimulation();
}

bool Game::isRunning()const{
    return !(/*m_window. || */m_interrupted);
}

void Game::end(){
    if (isRunning())
        m_interrupted = true;
}

Navigation * Game::navigation()
{
    return & m_navigation;
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
