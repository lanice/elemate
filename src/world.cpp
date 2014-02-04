#include "world.h"

#include <stdexcept>
#include <algorithm>

#include <glow/logging.h>
#include <glow/Program.h>
#include <glow/Shader.h>
#include <glowutils/File.h>
#include "cameraex.h"

#include <glm/glm.hpp>

#include "hpicgs/CyclicTime.h"
#include "physicswrapper.h"
#include "soundmanager.h"
#include "ui/navigation.h"
#include "elements.h"
#include "hand.h"
#include "terrain/terraingenerator.h"
#include "terrain/terrain.h"
#include "particlescriptaccess.h"
#include "particlegroup.h"
#include "lua/luawrapper.h"

World * World::s_instance = nullptr;

World::World(PhysicsWrapper & physicsWrapper)
: hand(nullptr)
, terrain(nullptr)
, m_soundManager(std::make_shared<SoundManager>())
, m_physicsWrapper(physicsWrapper)
, m_navigation(nullptr)
, m_time(std::make_shared<CyclicTime>(0.0L, 1.0L))
, m_sharedShaders()
, m_sounds()
, m_sunPosition(glm::normalize(glm::vec3(0.0, 6.5, 7.5)))
, m_sunlight()
{
    assert(s_instance == nullptr);

    // Create two non-3D channels (paino and rain)
    //initialise as paused
    m_soundManager->createNewChannel("data/sounds/rain.mp3", true, false, true);
    m_soundManager->createNewChannel("data/sounds/piano.mp3", true, false, true);
    //set volume (make quieter)
    m_soundManager->setVolume(0, 0.25f);
    m_soundManager->setVolume(1, 0.5f);

    initShader();

    TerrainGenerator terrainGen;
    terrain = std::shared_ptr<Terrain>(terrainGen.generate(*this));

    for (const auto actor : terrain->pxActorMap())
        m_physicsWrapper.addActor(*actor.second);

    hand = std::make_shared<Hand>(*this);

    m_sunlight[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    m_sunlight[1] = glm::vec4(0.2, 0.2, 0.2, 1.0);        //diffuse
    m_sunlight[2] = glm::vec4(0.7, 0.7, 0.5, 1.0);        //specular
    m_sunlight[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    m_skyColor = glm::vec3(0.6f, 0.9f, 1.f);

    ParticleScriptAccess::instance().setNotifier(this);
    
    s_instance = this;
}

World::~World()
{
    s_instance = nullptr;
}

World * World::instance()
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

void World::updatePhysics()
{
    // Retrieve time delta from last World update to now.
    double delta = static_cast<double>(m_time->getNonModf());
    delta = static_cast<double>(m_time->getNonModf(true)) - delta;

    if (delta == 0.0f)
        return;

    for (auto observer : m_particleGroupObservers)
        observer->updateEmitting(delta);

    // simulate physx
    m_physicsWrapper.step(delta);
}

void World::updateVisuals()
{
    updateListener();

    for (auto observer : m_particleGroupObservers)
        observer->updateVisuals();
}

void World::registerObserver(ParticleGroup * observer)
{
    m_particleGroupObservers.insert(observer);
}

void World::unregisterObserver(ParticleGroup * observer)
{
    m_particleGroupObservers.erase(observer);
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
    const CameraEx & cam = m_navigation->camera();
    glm::vec3 forward = glm::normalize(cam.eye() - cam.center());
    m_soundManager->setListenerAttributes(
    { cam.eye().x, cam.eye().y, cam.eye().z },
    { forward.x, forward.y, forward.z },
    { cam.up().x, cam.up().y, cam.up().z }
    );
    m_soundManager->update();
}

void World::setNavigation(Navigation & navigation)
{
    m_navigation = &navigation;
}

void World::initShader()
{
}

const glm::vec3 & World::sunPosition() const
{
    return m_sunPosition;
}

const glm::mat4 & World::sunlight() const
{
    return m_sunlight;
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
    program.setUniform("sunPosition", sunPosition());
    program.setUniform("sunlight", sunlight());
    program.setUniform("lightdir2", lightdir2);
    program.setUniform("light2", lightMat2);
    program.setUniform("skyColor", m_skyColor);
}

const glm::vec3 & World::skyColor() const
{
    return m_skyColor;
}

void World::registerLuaFunctions(LuaWrapper * lua)
{
    std::function<int()> func0 = [=] ()
    { togglePause(); return 0; };

    std::function<int(int)> func1 = [=] (int id)
    { toggleBackgroundSound(id); return 0; };

    lua->Register("world_togglePause", func0);
    lua->Register("world_toggleBackgroundSound", func1);
}

glow::Shader * World::sharedShader(GLenum type, const std::string & filename) const
{
    auto shaderIt = m_sharedShaders.find(filename);

    if (shaderIt != m_sharedShaders.end()) {
        assert(shaderIt->second->type() == type);
        return shaderIt->second.get();
    }

    glow::Shader * loaded = glowutils::createShaderFromFile(type, filename);
    m_sharedShaders.emplace(filename, loaded);

    return loaded;
}
