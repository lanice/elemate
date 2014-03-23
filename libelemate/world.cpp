#include "world.h"

#include <algorithm>

#include <glow/logging.h>
#include <glow/Program.h>
#include <glow/Shader.h>
#include <glowutils/global.h>
#include "utils/cameraex.h"

#include <glm/glm.hpp>

#include "utils/CyclicTime.h"
#include "physicswrapper.h"
#include "io/soundmanager.h"
#include "ui/navigation.h"
#include "ui/hand.h"
#include "terrain/terraingenerator.h"
#include "terrain/terrain.h"
#include "particles/particlegrouptycoon.h"
#include "particles/particlescriptaccess.h"
#include "particles/particlegroup.h"
#include "lua/luawrapper.h"
#include "texturemanager.h"
#include "ui/achievementmanager.h"

World * World::s_instance = nullptr;

World::World(PhysicsWrapper & physicsWrapper)
: hand(nullptr)
, terrain(nullptr)
, humidityFactor(-0.2f)
, m_physicsWrapper(physicsWrapper)
, m_time(std::make_shared<CyclicTime>(0.0L, 1.0L))
, m_sharedShaders()
, m_sounds()
, m_sunPosition(glm::normalize(glm::vec3(0.0, 6.5, 7.5)))
, m_sunlight()
, m_airHumidity(0)
, m_rainStrength(0.f)
, m_isRaining(false)
{
    assert(s_instance == nullptr);
    s_instance = this;

    SoundManager::initialize();
    // Create two non-3D channels (piano and rain)
    //initialize as paused
    int backgroundSoundId = SoundManager::instance()->createNewChannel("data/sounds/elemate.mp3", true, false, true);
    m_rainSoundId = SoundManager::instance()->createNewChannel("data/sounds/rain.mp3", true, false, true);
    //set volume (make quieter)
    SoundManager::instance()->setVolume(backgroundSoundId, 0.25f);
    SoundManager::instance()->setPaused(backgroundSoundId, false);

    AchievementManager::initialize();

    TextureManager::initialize();

    TerrainGenerator terrainGen;
    terrain = std::shared_ptr<Terrain>(terrainGen.generate());

    hand = std::make_shared<Hand>();

    m_sunlight[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    m_sunlight[1] = glm::vec4(0.2, 0.2, 0.2, 1.0);        //diffuse
    m_sunlight[2] = glm::vec4(0.7, 0.7, 0.5, 1.0);        //specular
    m_sunlight[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    ParticleGroupTycoon::initialize();
}

World::~World()
{
    TextureManager::release();
    ParticleGroupTycoon::release();
    SoundManager::release();
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
        SoundManager::instance()->setPaused(sound, !m_time->isRunning());

    for (auto observer : m_particleGroupObservers)
        observer->updateSounds(!m_time->isRunning());
}

time_t World::getTime() const
{
    return m_time->gett(false);
}

void World::updatePhysics()
{
    // Retrieve time delta from last World update to now.
    double delta = static_cast<double>(m_time->getNonModf());
    delta = static_cast<double>(m_time->getNonModf(true)) - delta;

    if (delta == 0.0f)
        return;

    terrain->updatePhysics(delta);

    ParticleGroupTycoon::instance().updatePhysics(delta);

    // simulate physx
    m_physicsWrapper.step(delta);

    if (m_isRaining)
    {
        unsigned int nextHumidity = static_cast<unsigned int>(m_airHumidity - 25000 * delta);
        m_airHumidity = nextHumidity > m_airHumidity ? 0 : nextHumidity;
        humidityFactor = (40.f - std::max(20.0f, 60.0f - m_airHumidity * 0.0001f)) * 0.01f;
        m_rainStrength = std::max(0.f, 1.f - 0.1f * (std::max(20.0f, 60.0f - m_airHumidity * 0.0001f) - 20.f));

        if (m_airHumidity < 100000)
        {
            fadeRainSound(static_cast<float>(m_airHumidity)/100000.f);
            if (m_airHumidity == 0) m_isRaining = false;
        }
    }
}

void World::updateVisuals(CameraEx & camera)
{
    updateListener(camera);

    ParticleGroupTycoon::instance().updateVisuals();
}

void World::toggleBackgroundSound(int id)
{
    SoundManager::instance()->togglePause(id);
}

void World::updateListener(const CameraEx & camera)
{
    glm::vec3 forward = glm::normalize(camera.eye() - camera.center());
    SoundManager::instance()->setListenerAttributes(camera.eye(), forward, camera.up());
    SoundManager::instance()->update();
}

void World::fadeRainSound(float intensity)
{
    SoundManager::instance()->setVolume(m_rainSoundId, intensity);
    intensity > 0.f ? SoundManager::instance()->setPaused(m_rainSoundId, false) : SoundManager::instance()->setPaused(m_rainSoundId, true);
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
}

void World::changeAirHumidity(int numSteamParticles)
{
    if (m_airHumidity > (unsigned int)(std::numeric_limits<int>::max()))
        m_airHumidity = std::numeric_limits<int>::max();
    m_airHumidity = std::max(0, int(m_airHumidity) + numSteamParticles);
    humidityFactor = (40.f - std::max(20.0f, 60.0f - m_airHumidity * 0.0001f)) * 0.01f;
    m_rainStrength = std::max(0.f, 1.f - 0.1f * (std::max(20.0f, 60.0f - m_airHumidity * 0.0001f) - 20.f));
    if (m_rainStrength >= 1.f) m_isRaining = true;
    AchievementManager::instance()->setProperty("rainStrength", m_rainStrength);
    if (m_rainStrength > 0) fadeRainSound(m_rainStrength);
}

float World::rainStrength() const
{
    if (m_isRaining) return 1.f;
    return m_rainStrength;
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
