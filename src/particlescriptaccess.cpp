#include "particlescriptaccess.h"

#include <cassert>

#include <glow/logging.h>

#include "particlegroup.h"
#include "world.h"
#include "lua/luawrapper.h"



ParticleScriptAccess ParticleScriptAccess::s_access;

ParticleScriptAccess::ParticleScriptAccess()
: m_worldNotifier(nullptr)
, m_gpuParticles(false)
{
}

ParticleScriptAccess::~ParticleScriptAccess()
{
}

ParticleScriptAccess& ParticleScriptAccess::instance()
{
    return s_access;
}

ParticleGroup * ParticleScriptAccess::particleGroup(const int index)
{
    return std::get<0>(m_particleGroups.at(index));
}

int ParticleScriptAccess::createParticleGroup(const std::string & elementType)
{
    ParticleGroup * particleGroup = new ParticleGroup(m_gpuParticles);
    LuaWrapper * wrapper = new LuaWrapper();

    setUpParticleGroup(particleGroup, wrapper, elementType);

    if (m_freeIndices.size() > 0)
    {
        int i = m_freeIndices.back();
        assert(std::get<0>(m_particleGroups.at(i)) == nullptr && std::get<1>(m_particleGroups.at(i)) == nullptr);
        m_freeIndices.pop_back();
        m_particleGroups.at(i) = std::make_tuple(particleGroup, wrapper, elementType);
        return i;
    }

    m_particleGroups.push_back(std::make_tuple(particleGroup, wrapper, elementType));
    m_worldNotifier->registerObserver(particleGroup);
    return static_cast<int>(m_particleGroups.size() - 1);
}

void ParticleScriptAccess::removeParticleGroup(const int index)
{
    delete std::get<0>(m_particleGroups.at(index));
    delete std::get<1>(m_particleGroups.at(index));

    std::get<0>(m_particleGroups.at(index)) = nullptr;
    std::get<1>(m_particleGroups.at(index)) = nullptr;

    m_freeIndices.push_back(index);
}

void ParticleScriptAccess::setUpParticleGroup(ParticleGroup * particleGroup, LuaWrapper * wrapper, const std::string & elementType)
{
    std::function<int(float, float, float, float, float)> func1 = [=](float a, float b, float c, float d, float e) {particleGroup->setImmutableProperties(a, b, c, d, e); return 0; };

    std::function<int(float, float, float, float, float, float, float)> func2 = [=](float a, float b, float c, float d, float e, float f, float g) {particleGroup->setMutableProperties(a, b, c, d, e, f, g); return 0; };

    wrapper->Register("particles_setImmutableProperties", func1);
    wrapper->Register("particles_setMutableProperties", func2);

    std::string script = "scripts/elements/" + elementType + ".lua";
    wrapper->loadScript(script);
}

void ParticleScriptAccess::setNotifier(World * notifier)
{
    m_worldNotifier = notifier;
}

void ParticleScriptAccess::setUseGpuParticles(bool enable)
{
    if (m_gpuParticles == enable) return;

    m_gpuParticles = enable;

    for (auto particleGroupTuple : m_particleGroups)
        std::get<0>(particleGroupTuple)->setUseGpuParticles(enable);
}

void ParticleScriptAccess::pauseGPUAcceleration()
{
    assert(m_gpuParticlesPauseFlags == 0x00);
    if ((m_gpuParticlesPauseFlags & 0x01) == 0x01) // break, if already paused
        return;

    m_gpuParticlesPauseFlags = 0x01 | (m_gpuParticles ? 0x10 : 0x00);

    if (!m_gpuParticles)
        return;
    setUseGpuParticles(false);
}

void ParticleScriptAccess::restoreGPUAccelerated()
{
    assert((m_gpuParticlesPauseFlags & 0x01) == 0x01);
    if ((m_gpuParticlesPauseFlags & 0x01) == 0x00)   // break, if not paused
        return;

    if ((m_gpuParticlesPauseFlags & 0x10) == 0x10)
        setUseGpuParticles(true);

    m_gpuParticlesPauseFlags = 0x00;
}

void ParticleScriptAccess::registerLuaFunctions(LuaWrapper * lua)
{
    std::function<int(std::string)> func0 = [=] (std::string elementType)
    { return createParticleGroup(elementType); };

    std::function<int(int, float, float, float, float, float, float)> func1 = [=] (int index, float posx, float posy, float posz, float velx, float vely, float velz)
    { createParticle(index, posx, posy, posz, velx, vely, velz); return 0; };

    std::function<int(int, float, float, float, float, float, float, float)> func2 = [=] (int index, float ratio, float posx, float posy, float posz, float dirx, float diry, float dirz)
    { emit(index, ratio, posx, posy, posz, dirx, diry, dirz); return 0; };

    std::function<int(int)> func3 = [=] (int index)
    { stopEmit(index); return 0; };

    std::function<int()> func4 = [=] ()
    { return numParticleGroups(); };

    std::function<std::string(int)> func5 = [=] (int index)
    { return elementAtIndex(index); };

    lua->Register("psa_createParticleGroup", func0);
    lua->Register("psa_createParticle", func1);
    lua->Register("psa_emit", func2);
    lua->Register("psa_stopEmit", func3);
    lua->Register("psa_numParticleGroups", func4);
    lua->Register("psa_elementAtIndex", func5);
}

void ParticleScriptAccess::createParticle(const int index, const float positionX, const float positionY, const float positionZ, const float velocityX, const float velocityY, const float velocityZ)
{
    std::get<0>(m_particleGroups.at(index))->createParticle(glm::vec3(positionX, positionY, positionZ), glm::vec3(velocityX, velocityY, velocityZ));
}

void ParticleScriptAccess::emit(const int index, const float ratio, const float positionX, const float positionY, const float positionZ, const float directionX, const float directionY, const float directionZ)
{
    std::get<0>(m_particleGroups.at(index))->emit(ratio, glm::vec3(positionX, positionY, positionZ), glm::vec3(directionX, directionY, directionZ));
}

void ParticleScriptAccess::stopEmit(const int index)
{
    std::get<0>(m_particleGroups.at(index))->stopEmit();
}

void ParticleScriptAccess::setImmutableProperties( const int index, const float maxMotionDistance, const float gridSize, const float restOffset, const float contactOffset, const float restParticleDistance)
{
    std::get<0>(m_particleGroups.at(index))->setImmutableProperties(maxMotionDistance, gridSize, restOffset, contactOffset, restParticleDistance);
}

void ParticleScriptAccess::setMutableProperties(const int index, const float restitution, const float dynamicFriction, const float staticFriction, const float damping, const float particleMass, const float viscosity, const float stiffness)
{
    std::get<0>(m_particleGroups.at(index))->setMutableProperties(restitution, dynamicFriction, staticFriction, damping, particleMass, viscosity, stiffness);
}

int ParticleScriptAccess::numParticleGroups()
{
    return static_cast<int>(m_particleGroups.size() - 1);
}

std::string ParticleScriptAccess::elementAtIndex(int index)
{
    return std::get<2>(m_particleGroups.at(index));
}
