#include "particlescriptaccess.h"

#include <cassert>

#include <glow/logging.h>

#include "utils/pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxScene.h>
#include <PxSceneLock.h>

#include "particlegroup.h"
#include "particlecollision.h"
#include "world.h"
#include "lua/luawrapper.h"


ParticleScriptAccess * ParticleScriptAccess::s_instance = nullptr;

void ParticleScriptAccess::initialize(World & notifier)
{
    assert(s_instance == nullptr);
    s_instance = new ParticleScriptAccess();
    s_instance->m_worldNotifier = &notifier;
}

void ParticleScriptAccess::release()
{
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

ParticleScriptAccess::ParticleScriptAccess()
: m_id(0)
, m_collisions(std::make_shared<ParticleCollision>(*this))
, m_collisionCheckDelta(0.0)
, m_worldNotifier(nullptr)
, m_gpuParticles(false)
, m_lua(nullptr)
, m_pxScene(nullptr)
{
    assert(PxGetPhysics().getNbScenes() == 1);
    physx::PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    m_pxScene = pxScenePtrs[0];

    m_lua = new LuaWrapper();

    std::function<int(int, float, float, float, float, float)> func1 = [=] (int id, float a, float b, float c, float d, float e)
    { particleGroup(id)->setImmutableProperties(a, b, c, d, e); return 0; };

    std::function<int(int, float, float, float, float, float, float, float)> func2 = [=] (int id, float a, float b, float c, float d, float e, float f, float g)
    { particleGroup(id)->setMutableProperties(a, b, c, d, e, f, g); return 0; };

    m_lua->Register("psa_setImmutableProperties", func1);
    m_lua->Register("psa_setMutableProperties", func2);
}

ParticleScriptAccess::~ParticleScriptAccess()
{
    delete m_lua;
}

ParticleScriptAccess& ParticleScriptAccess::instance()
{
    return *s_instance;
}

void ParticleScriptAccess::checkCollisions(double deltaTime)
{
    m_collisionCheckDelta += deltaTime;
    /*if (m_collisionCheckDelta > 0.5)*/ {
        m_collisions->performCheck();
        m_collisionCheckDelta = 0.0;
    }
}

ParticleGroup * ParticleScriptAccess::particleGroup(const int id)
{
    return m_particleGroups.at(id);
}

int ParticleScriptAccess::createParticleGroup(const std::string & elementType, uint32_t maxParticleCount)
{
    ParticleGroup * particleGroup = new ParticleGroup(elementType, m_gpuParticles, maxParticleCount);

    m_particleGroups.emplace(m_id, particleGroup);

    setUpParticleGroup(m_id, elementType);
    m_worldNotifier->registerObserver(particleGroup);

    return m_id++;
}

void ParticleScriptAccess::removeParticleGroup(const int id)
{
    m_worldNotifier->unregisterObserver(m_particleGroups.at(id));

    m_collisions->particleGroupDeleted(m_particleGroups.at(id)->elementName(), id);

    delete m_particleGroups.at(id);
    m_particleGroups.erase(id);
}

void ParticleScriptAccess::clearParticleGroups()
{
    for (auto it = m_particleGroups.begin(); it != m_particleGroups.end(); ++it)
    {
        m_worldNotifier->unregisterObserver(it->second);
        delete it->second;
    }

    m_particleGroups.erase(m_particleGroups.begin(), m_particleGroups.end());
}

void ParticleScriptAccess::setUpParticleGroup(const int id, const std::string & elementType)
{
    std::string script = "scripts/elements/" + elementType + ".lua";
    m_lua->loadScript(script);
    m_lua->call("setImmutableProperties", id);
    m_lua->call("setMutableProperties", id);
    m_lua->removeScript(script);
}

void ParticleScriptAccess::setUseGpuParticles(bool enable)
{
    if (m_gpuParticles == enable) return;

    m_gpuParticles = enable;

    for (auto particleGroup : m_particleGroups)
        particleGroup.second->setUseGpuParticles(enable);
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
    std::function<int(std::string, unsigned int)> func0 = [=] (std::string elementType, unsigned int maxParticles)
    { return createParticleGroup(elementType, maxParticles); };

    std::function<int(int)> func0a = [=] (int id)
    { removeParticleGroup(id); return 0; };

    std::function<int()> func0b = [=] ()
    { clearParticleGroups(); return 0; };

    std::function<int(int, float, float, float, float, float, float)> func1 = [=] (int id, float posx, float posy, float posz, float velx, float vely, float velz)
    { createParticle(id, posx, posy, posz, velx, vely, velz); return 0; };

    std::function<int(int, float, float, float, float, float, float, float)> func2 = [=] (int id, float ratio, float posx, float posy, float posz, float dirx, float diry, float dirz)
    { emit(id, ratio, posx, posy, posz, dirx, diry, dirz); return 0; };

    std::function<int(int)> func3 = [=] (int id)
    { stopEmit(id); return 0; };

    std::function<int()> func4 = [=] ()
    { return numParticleGroups(); };

    std::function<std::string(int)> func5 = [=] (int id)
    { return elementAtId(id); };

    std::function<int(int)> func5a = [=] (int id)
    { return nextParticleGroup(id); };

    lua->Register("psa_createParticleGroup", func0);
    lua->Register("psa_removeParticleGroup", func0a);
    lua->Register("psa_clearParticleGroups", func0b);
    lua->Register("psa_createParticle", func1);
    lua->Register("psa_emit", func2);
    lua->Register("psa_stopEmit", func3);
    lua->Register("psa_numParticleGroups", func4);
    lua->Register("psa_elementAtId", func5);
    lua->Register("psa_nextParticleGroup", func5a);

    std::function<int(int, float)> func6 = [=] (int id, float maxMotionDistance)
    { setMaxMotionDistance(id, maxMotionDistance); return 0; };
    std::function<int(int, float)> func7 = [=] (int id, float gridSize)
    { setGridSize(id, gridSize); return 0; };
    std::function<int(int, float)> func8 = [=] (int id, float restOffset)
    { setRestOffset(id, restOffset); return 0; };
    std::function<int(int, float)> func9 = [=] (int id, float contactOffset)
    { setContactOffset(id, contactOffset); return 0; };
    std::function<int(int, float)> func10 = [=] (int id, float restParticleDistance)
    { setRestParticleDistance(id, restParticleDistance); return 0; };

    std::function<int(int, float)> func11 = [=] (int id, float restitution)
    { setRestitution(id, restitution); return 0; };
    std::function<int(int, float)> func12 = [=] (int id, float dynamicFriction)
    { setDynamicFriction(id, dynamicFriction); return 0; };
    std::function<int(int, float)> func13 = [=] (int id, float staticFriction)
    { setStaticFriction(id, staticFriction); return 0; };
    std::function<int(int, float)> func14 = [=] (int id, float damping)
    { setDamping(id, damping); return 0; };
    std::function<int(int, float)> func15 = [=] (int id, float particleMass)
    { setParticleMass(id, particleMass); return 0; };
    std::function<int(int, float)> func16 = [=] (int id, float viscosity)
    { setViscosity(id, viscosity); return 0; };
    std::function<int(int, float)> func17 = [=] (int id, float stiffness)
    { setStiffness(id, stiffness); return 0; };

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

    std::function<float(int)> func18 = [=] (int id)
    { return maxMotionDistance(id); };
    std::function<float(int)> func19 = [=] (int id)
    { return gridSize(id); };
    std::function<float(int)> func20 = [=] (int id)
    { return restOffset(id); };
    std::function<float(int)> func21 = [=] (int id)
    { return contactOffset(id); };
    std::function<float(int)> func22 = [=] (int id)
    { return restParticleDistance(id); };
    std::function<float(int)> func23 = [=] (int id)
    { return restitution(id); };
    std::function<float(int)> func24 = [=] (int id)
    { return dynamicFriction(id); };
    std::function<float(int)> func25 = [=] (int id)
    { return staticFriction(id); };
    std::function<float(int)> func26 = [=] (int id)
    { return damping(id); };
    std::function<float(int)> func27 = [=] (int id)
    { return particleMass(id); };
    std::function<float(int)> func28 = [=] (int id)
    { return viscosity(id); };
    std::function<float(int)> func29 = [=] (int id)
    { return stiffness(id); };

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


    std::function<int(int, float, float, float, float, float)> func30 = [=](int id, float a, float b, float c, float d, float e)
    { particleGroup(id)->setImmutableProperties(a, b, c, d, e); return 0; };

    std::function<int(int, float, float, float, float, float, float, float)> func31 = [=](int id, float a, float b, float c, float d, float e, float f, float g)
    { particleGroup(id)->setMutableProperties(a, b, c, d, e, f, g); return 0; };

    lua->Register("psa_setImmutableProperties", func30);
    lua->Register("psa_setMutableProperties", func31);
}

void ParticleScriptAccess::createParticle(const int id, const float positionX, const float positionY, const float positionZ, const float velocityX, const float velocityY, const float velocityZ)
{
    m_particleGroups.at(id)->createParticle(glm::vec3(positionX, positionY, positionZ), glm::vec3(velocityX, velocityY, velocityZ));
}

void ParticleScriptAccess::emit(const int id, const float ratio, const float positionX, const float positionY, const float positionZ, const float directionX, const float directionY, const float directionZ)
{
    m_particleGroups.at(id)->emit(ratio, glm::vec3(positionX, positionY, positionZ), glm::vec3(directionX, directionY, directionZ));
}

void ParticleScriptAccess::stopEmit(const int id)
{
    m_particleGroups.at(id)->stopEmit();
}

void ParticleScriptAccess::setImmutableProperties( const int id, const float maxMotionDistance, const float gridSize, const float restOffset, const float contactOffset, const float restParticleDistance)
{
    m_particleGroups.at(id)->setImmutableProperties(maxMotionDistance, gridSize, restOffset, contactOffset, restParticleDistance);
}

void ParticleScriptAccess::setMutableProperties(const int id, const float restitution, const float dynamicFriction, const float staticFriction, const float damping, const float particleMass, const float viscosity, const float stiffness)
{
    m_particleGroups.at(id)->setMutableProperties(restitution, dynamicFriction, staticFriction, damping, particleMass, viscosity, stiffness);
}

int ParticleScriptAccess::numParticleGroups()
{
    return static_cast<int>(m_particleGroups.size() - 1);
}

const std::string & ParticleScriptAccess::elementAtId(int id)
{
    return m_particleGroups.at(id)->elementName();
}

int ParticleScriptAccess::nextParticleGroup(int id)
{
    auto it = m_particleGroups.find(id);

    if (it == m_particleGroups.end())
    {
        if (m_particleGroups.begin() == m_particleGroups.end())
            return -1;
        else
            return m_particleGroups.begin()->first;
    }

    if (++it == m_particleGroups.end())
        return m_particleGroups.begin()->first;

    return it->first;
}

void ParticleScriptAccess::setMaxMotionDistance(int id, float maxMotionDistance)
{
    m_pxScene->removeActor(*m_particleGroups.at(id)->particleSystem());
    m_particleGroups.at(id)->particleSystem()->setMaxMotionDistance(maxMotionDistance);
    m_pxScene->addActor(*m_particleGroups.at(id)->particleSystem());
}
void ParticleScriptAccess::setGridSize(int id, float gridSize)
{
    m_pxScene->removeActor(*m_particleGroups.at(id)->particleSystem());
    m_particleGroups.at(id)->particleSystem()->setGridSize(gridSize);
    m_pxScene->addActor(*m_particleGroups.at(id)->particleSystem());
}
void ParticleScriptAccess::setRestOffset(int id, float restOffset)
{
    m_pxScene->removeActor(*m_particleGroups.at(id)->particleSystem());
    m_particleGroups.at(id)->particleSystem()->setRestOffset(restOffset);
    m_pxScene->addActor(*m_particleGroups.at(id)->particleSystem());
}
void ParticleScriptAccess::setContactOffset(int id, float contactOffset)
{
    m_pxScene->removeActor(*m_particleGroups.at(id)->particleSystem());
    m_particleGroups.at(id)->particleSystem()->setContactOffset(contactOffset);
    m_pxScene->addActor(*m_particleGroups.at(id)->particleSystem());
}
void ParticleScriptAccess::setRestParticleDistance(int id, float restParticleDistance)
{
    m_pxScene->removeActor(*m_particleGroups.at(id)->particleSystem());
    m_particleGroups.at(id)->particleSystem()->setRestParticleDistance(restParticleDistance);
    m_pxScene->addActor(*m_particleGroups.at(id)->particleSystem());
}
void ParticleScriptAccess::setRestitution(int id, float restitution)
{ m_particleGroups.at(id)->particleSystem()->setRestitution(restitution); }
void ParticleScriptAccess::setDynamicFriction(int id, float dynamicFriction)
{ m_particleGroups.at(id)->particleSystem()->setDynamicFriction(dynamicFriction); }
void ParticleScriptAccess::setStaticFriction(int id, float staticFriction)
{ m_particleGroups.at(id)->particleSystem()->setStaticFriction(staticFriction); }
void ParticleScriptAccess::setDamping(int id, float damping)
{ m_particleGroups.at(id)->particleSystem()->setDamping(damping); }
void ParticleScriptAccess::setParticleMass(int id, float particleMass)
{ m_particleGroups.at(id)->particleSystem()->setParticleMass(particleMass); }
void ParticleScriptAccess::setViscosity(int id, float viscosity)
{ m_particleGroups.at(id)->particleSystem()->setViscosity(viscosity); }
void ParticleScriptAccess::setStiffness(int id, float stiffness)
{ m_particleGroups.at(id)->particleSystem()->setStiffness(stiffness); }
float ParticleScriptAccess::maxMotionDistance(int id)
{ return m_particleGroups.at(id)->particleSystem()->getMaxMotionDistance(); }
float ParticleScriptAccess::gridSize(int id)
{ return m_particleGroups.at(id)->particleSystem()->getGridSize(); }
float ParticleScriptAccess::restOffset(int id)
{ return m_particleGroups.at(id)->particleSystem()->getRestOffset(); }
float ParticleScriptAccess::contactOffset(int id)
{ return m_particleGroups.at(id)->particleSystem()->getContactOffset(); }
float ParticleScriptAccess::restParticleDistance(int id)
{ return m_particleGroups.at(id)->particleSystem()->getRestParticleDistance(); }
float ParticleScriptAccess::restitution(int id)
{ return m_particleGroups.at(id)->particleSystem()->getRestitution(); }
float ParticleScriptAccess::dynamicFriction(int id)
{ return m_particleGroups.at(id)->particleSystem()->getDynamicFriction(); }
float ParticleScriptAccess::staticFriction(int id)
{ return m_particleGroups.at(id)->particleSystem()->getStaticFriction(); }
float ParticleScriptAccess::damping(int id)
{ return m_particleGroups.at(id)->particleSystem()->getDamping(); }
float ParticleScriptAccess::particleMass(int id)
{ return m_particleGroups.at(id)->particleSystem()->getParticleMass(); }
float ParticleScriptAccess::viscosity(int id)
{ return m_particleGroups.at(id)->particleSystem()->getViscosity(); }
float ParticleScriptAccess::stiffness(int id)
{ return m_particleGroups.at(id)->particleSystem()->getStiffness(); }
