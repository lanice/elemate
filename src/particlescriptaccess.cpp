#include "particlescriptaccess.h"

#include <cassert>

#include <glow/logging.h>

#include "pxcompilerfix.h"
#include <PxScene.h>
#include <PxSceneLock.h>

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

    std::function<int(int, float)> func6 = [=] (int index, float maxMotionDistance)
    { setMaxMotionDistance(index, maxMotionDistance); return 0; };
    std::function<int(int, float)> func7 = [=] (int index, float gridSize)
    { setGridSize(index, gridSize); return 0; };
    std::function<int(int, float)> func8 = [=] (int index, float restOffset)
    { setRestOffset(index, restOffset); return 0; };
    std::function<int(int, float)> func9 = [=] (int index, float contactOffset)
    { setContactOffset(index, contactOffset); return 0; };
    std::function<int(int, float)> func10 = [=] (int index, float restParticleDistance)
    { setRestParticleDistance(index, restParticleDistance); return 0; };

    std::function<int(int, float)> func11 = [=] (int index, float restitution)
    { setRestitution(index, restitution); return 0; };
    std::function<int(int, float)> func12 = [=] (int index, float dynamicFriction)
    { setDynamicFriction(index, dynamicFriction); return 0; };
    std::function<int(int, float)> func13 = [=] (int index, float staticFriction)
    { setStaticFriction(index, staticFriction); return 0; };
    std::function<int(int, float)> func14 = [=] (int index, float damping)
    { setDamping(index, damping); return 0; };
    std::function<int(int, float)> func15 = [=] (int index, float particleMass)
    { setParticleMass(index, particleMass); return 0; };
    std::function<int(int, float)> func16 = [=] (int index, float viscosity)
    { setViscosity(index, viscosity); return 0; };
    std::function<int(int, float)> func17 = [=] (int index, float stiffness)
    { setStiffness(index, stiffness); return 0; };

    lua->Register("psa_setMaxMotionDistance", func6);
    lua->Register("psa_setGridSize", func7);
    lua->Register("psa_setRestOffset", func8);
    lua->Register("psa_setContactOffset", func9);
    lua->Register("psa_setRestParticleDistance", func10);
    lua->Register("psa_setRestitution", func11);
    lua->Register("psa_setDynamicFriction", func12);
    lua->Register("psa_setStaticFriction", func13);
    lua->Register("psa_setDamping", func14);
    lua->Register("psa_setParticleMass", func15);
    lua->Register("psa_setViscosity", func16);
    lua->Register("psa_setStiffness", func17);

    std::function<float(int)> func18 = [=] (int index)
    { return maxMotionDistance(index); };
    std::function<float(int)> func19 = [=] (int index)
    { return gridSize(index); };
    std::function<float(int)> func20 = [=] (int index)
    { return restOffset(index); };
    std::function<float(int)> func21 = [=] (int index)
    { return contactOffset(index); };
    std::function<float(int)> func22 = [=] (int index)
    { return restParticleDistance(index); };
    std::function<float(int)> func23 = [=] (int index)
    { return restitution(index); };
    std::function<float(int)> func24 = [=] (int index)
    { return dynamicFriction(index); };
    std::function<float(int)> func25 = [=] (int index)
    { return staticFriction(index); };
    std::function<float(int)> func26 = [=] (int index)
    { return damping(index); };
    std::function<float(int)> func27 = [=] (int index)
    { return particleMass(index); };
    std::function<float(int)> func28 = [=] (int index)
    { return viscosity(index); };
    std::function<float(int)> func29 = [=] (int index)
    { return stiffness(index); };

    lua->Register("psa_maxMotionDistance", func18);
    lua->Register("psa_gridSize", func19);
    lua->Register("psa_restOffset", func20);
    lua->Register("psa_contactOffset", func21);
    lua->Register("psa_restParticleDistance", func22);
    lua->Register("psa_restitution", func23);
    lua->Register("psa_dynamicFriction", func24);
    lua->Register("psa_staticFriction", func25);
    lua->Register("psa_damping", func26);
    lua->Register("psa_particleMass", func27);
    lua->Register("psa_viscosity", func28);
    lua->Register("psa_stiffness", func29);

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

void ParticleScriptAccess::setMaxMotionDistance(int index, float maxMotionDistance)
{
    std::get<0>(m_particleGroups.at(index))->physxScene()->removeActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
    std::get<0>(m_particleGroups.at(index))->particleSystem()->setMaxMotionDistance(maxMotionDistance);
    std::get<0>(m_particleGroups.at(index))->physxScene()->addActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
}
void ParticleScriptAccess::setGridSize(int index, float gridSize)
{
    std::get<0>(m_particleGroups.at(index))->physxScene()->removeActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
    std::get<0>(m_particleGroups.at(index))->particleSystem()->setGridSize(gridSize);
    std::get<0>(m_particleGroups.at(index))->physxScene()->addActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
}
void ParticleScriptAccess::setRestOffset(int index, float restOffset)
{
    std::get<0>(m_particleGroups.at(index))->physxScene()->removeActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
    std::get<0>(m_particleGroups.at(index))->particleSystem()->setRestOffset(restOffset);
    std::get<0>(m_particleGroups.at(index))->physxScene()->addActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
}
void ParticleScriptAccess::setContactOffset(int index, float contactOffset)
{
    std::get<0>(m_particleGroups.at(index))->physxScene()->removeActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
    std::get<0>(m_particleGroups.at(index))->particleSystem()->setContactOffset(contactOffset);
    std::get<0>(m_particleGroups.at(index))->physxScene()->addActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
}
void ParticleScriptAccess::setRestParticleDistance(int index, float restParticleDistance)
{
    std::get<0>(m_particleGroups.at(index))->physxScene()->removeActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
    std::get<0>(m_particleGroups.at(index))->particleSystem()->setRestParticleDistance(restParticleDistance);
    std::get<0>(m_particleGroups.at(index))->physxScene()->addActor(*std::get<0>(m_particleGroups.at(index))->particleSystem());
}
void ParticleScriptAccess::setRestitution(int index, float restitution)
{ std::get<0>(m_particleGroups.at(index))->particleSystem()->setRestitution(restitution); }
void ParticleScriptAccess::setDynamicFriction(int index, float dynamicFriction)
{ std::get<0>(m_particleGroups.at(index))->particleSystem()->setDynamicFriction(dynamicFriction); }
void ParticleScriptAccess::setStaticFriction(int index, float staticFriction)
{ std::get<0>(m_particleGroups.at(index))->particleSystem()->setStaticFriction(staticFriction); }
void ParticleScriptAccess::setDamping(int index, float damping)
{ std::get<0>(m_particleGroups.at(index))->particleSystem()->setDamping(damping); }
void ParticleScriptAccess::setParticleMass(int index, float particleMass)
{ std::get<0>(m_particleGroups.at(index))->particleSystem()->setParticleMass(particleMass); }
void ParticleScriptAccess::setViscosity(int index, float viscosity)
{ std::get<0>(m_particleGroups.at(index))->particleSystem()->setViscosity(viscosity); }
void ParticleScriptAccess::setStiffness(int index, float stiffness)
{ std::get<0>(m_particleGroups.at(index))->particleSystem()->setStiffness(stiffness); }
float ParticleScriptAccess::maxMotionDistance(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getMaxMotionDistance(); }
float ParticleScriptAccess::gridSize(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getGridSize(); }
float ParticleScriptAccess::restOffset(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getRestOffset(); }
float ParticleScriptAccess::contactOffset(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getContactOffset(); }
float ParticleScriptAccess::restParticleDistance(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getRestParticleDistance(); }
float ParticleScriptAccess::restitution(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getRestitution(); }
float ParticleScriptAccess::dynamicFriction(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getDynamicFriction(); }
float ParticleScriptAccess::staticFriction(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getStaticFriction(); }
float ParticleScriptAccess::damping(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getDamping(); }
float ParticleScriptAccess::particleMass(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getParticleMass(); }
float ParticleScriptAccess::viscosity(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getViscosity(); }
float ParticleScriptAccess::stiffness(int index)
{ return std::get<0>(m_particleGroups.at(index))->particleSystem()->getStiffness(); }
