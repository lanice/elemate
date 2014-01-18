#include "world.h"

#include <stdexcept>

#include <glow/logging.h>
#include <glow/Program.h>
#include <glow/Shader.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include <glm/glm.hpp>

#include "hpicgs/CyclicTime.h"
#include "physicswrapper.h"
#include "soundmanager.h"
#include "ui/navigation.h"
#include "elements.h"
#include "hand.h"
#include "terrain/terraingenerator.h"
#include "terrain/terrain.h"

World * World::s_instance = nullptr;

World::World(PhysicsWrapper & physicsWrapper)
: hand(nullptr)
, terrain(nullptr)
, m_soundManager(std::make_shared<SoundManager>())
, m_physicsWrapper(physicsWrapper)
, m_navigation(nullptr)
, m_time(std::make_shared<CyclicTime>(0.0L, 1.0L))
, m_programsByName()
, m_sounds()
, m_sunlightInvDirection(glm::vec3(0.0, 6.5, 7.5))
, m_sunlighting()
{
    // Create two non-3D channels (paino and rain)
    //initialise as paused
    m_soundManager->createNewChannel("data/sounds/rain.mp3", true, false, true);
    m_soundManager->createNewChannel("data/sounds/piano.mp3", true, false, true);
    //set volume (make quieter)
    m_soundManager->setVolume(0, 0.25f);
    m_soundManager->setVolume(1, 0.5f);

    initShader();

    TerrainGenerator terrainGen;
    terrainGen.setExtentsInWorld(50, 50);
    terrainGen.applySamplesPerWorldCoord(3.f);
    terrainGen.setTilesPerAxis(1, 1);
    terrainGen.setMaxHeight(20.0f);
    terrainGen.setMaxBasicHeightVariance(0.05f);

    terrain = std::shared_ptr<Terrain>(terrainGen.generate(*this));

    for (const auto actor : terrain->pxActorMap())
        m_physicsWrapper.addActor(*actor.second);

    hand = std::make_shared<Hand>(*this);

    m_sunlighting[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    m_sunlighting[1] = glm::vec4(0.2, 0.2, 0.2, 1.0);        //diffuse
    m_sunlighting[2] = glm::vec4(0.7, 0.7, 0.5, 1.0);        //specular
    m_sunlighting[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    assert(s_instance == nullptr);
    s_instance = this;
}

World::~World()
{
}

World * World::getInstance()
{
    assert(s_instance);
    return s_instance;
}

void World::togglePause()
{
    m_time->isRunning() ? m_time->pause() : m_time->start();

    // Pause/resume all sounds except the background sounds.
    for (const auto sound : m_sounds)
        m_soundManager->setPaused(sound, !m_time->isRunning());
}

void World::stopSimulation()
{
    m_time->stop(true);
}

void World::update()
{
    // Retrieve time delta from last World update to now.
    double delta = static_cast<double>(m_time->getNonModf());
    delta = static_cast<double>(m_time->getNonModf(true)) - delta;

    // update physic
    m_physicsWrapper.step(delta);
}

void World::makeElements(const glm::vec3& position)
{
    m_physicsWrapper.clearEmitters();
    m_currentElements = Elements::availableElements();
    for (const auto& element_name: m_currentElements)
        m_physicsWrapper.makeParticleEmitter(element_name, position);
    selectNextEmitter();
}



void World::createFountainSound(const glm::vec3& position)
{
    int id = m_soundManager->createNewChannel("data/sounds/fountain_loop.wav", true, true, !m_time->isRunning(), { position.x, position.y, position.z });
    m_sounds.push_back(id);
}

void World::toggleBackgroundSound(int id){
    m_soundManager->togglePause(id);
}

void World::updateListener(){
    auto cam = m_navigation->camera();
    glm::vec3 forward = glm::normalize(cam->eye() - cam->center());
    m_soundManager->setListenerAttributes(
    { cam->eye().x, cam->eye().y, cam->eye().z },
    { forward.x, forward.y, forward.z },
    { cam->up().x, cam->up().y, cam->up().z }
    );
    m_soundManager->update();
}

void World::reloadLua()
{
    m_physicsWrapper.reloadLua();
}

void World::setNavigation(Navigation & navigation)
{
    m_navigation = &navigation;
}

void World::initShader()
{
}

const glm::vec3 & World::sunlightInvDirection() const
{
    return m_sunlightInvDirection;
}

const glm::mat4 & World::sunlighting() const
{
    return m_sunlighting;
}

void World::setUpLighting(glow::Program & program) const
{
    static glm::vec4 lightambientglobal(0, 0, 0, 0);
    static glm::vec3 lightdir2(0.0, -8.0, 7.5);

    static glm::mat4 lightMat2;
    lightMat2[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    lightMat2[1] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //diffuse
    lightMat2[2] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //specular
    lightMat2[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    program.setUniform("lightambientglobal", lightambientglobal);
    program.setUniform("sunlightInvDir", sunlightInvDirection());
    program.setUniform("sunlighting", sunlighting());
    program.setUniform("lightdir2", lightdir2);
    program.setUniform("light2", lightMat2);
}

glow::Program * World::programByName(const std::string & name)
{
    try {
        return m_programsByName.at(name).get();
    }
    catch (std::out_of_range e) {
        glow::critical("trying to use unloaded shader %;", name);
        return nullptr;
    }
}

void World::updateEmitterPosition(const glm::vec3& position)
{
    m_physicsWrapper.updateEmitterPosition(position);
    //FMOD_VECTOR pos;
    //pos.x = position.x; 
    //pos.y = position.y;
    //pos.z = position.z;
    //m_soundManager->setSoundPos(0,pos); // Not good .... just for testing reasons
}

void World::selectNextEmitter()
{
    m_currentElements.splice(m_currentElements.end(), m_currentElements, m_currentElements.begin());
    m_physicsWrapper.selectEmitter(m_currentElements.front());
}

void World::startEmitting()
{
    m_physicsWrapper.startEmitting();
}

void World::stopEmitting()
{
    m_physicsWrapper.stopEmitting();
}
