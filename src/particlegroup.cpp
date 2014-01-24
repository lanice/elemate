#include "particlegroup.h"

#include <cassert>

#include <glow/logging.h>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxSceneLock.h>

// #include "PxPhysicsAPI.h"

#include "particledrawable.h"


using namespace physx;

ParticleGroup::ParticleGroup(
    const PxU32 maxParticleCount,
    const ImmutableParticleProperties & immutableProperties,
    const MutableParticleProperties & mutableProperties
    )
: m_particleSystem(nullptr)
, m_scene(nullptr)
, m_particleDrawable(std::make_shared<ParticleDrawable>(maxParticleCount))
, m_indices(new PxU32[maxParticleCount]())
, m_nextFreeIndex(0)
{
    for (PxU32 i = 0; i < maxParticleCount; ++i) m_indices[i] = i;

    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    m_scene = pxScenePtrs[0];

    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem = PxGetPhysics().createParticleFluid(maxParticleCount, false);
    assert(m_particleSystem);
    // m_particleSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, m_gpuParticles);

    m_scene->addActor(*m_particleSystem);
    
    setImmutableProperties(immutableProperties);
    setMutableProperties(mutableProperties);
}

ParticleGroup::~ParticleGroup()
{
    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem->releaseParticles();
    m_scene->removeActor(*m_particleSystem);
    m_particleSystem = nullptr;
}

void ParticleGroup::createParticles(const PxU32 numParticles, const PxVec3 * positions, const PxVec3 * velocities)
{
    PxU32 indices[numParticles];

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        if (m_freeIndices.size() > 0)
        {
            indices[i] = m_freeIndices.back();
            m_freeIndices.pop_back();
        } else {
            indices[i] = m_nextFreeIndex++;
        }
    }

    PxParticleCreationData particleCreationData;
    particleCreationData.numParticles = numParticles;
    particleCreationData.indexBuffer = PxStrideIterator<const PxU32>(indices);
    particleCreationData.positionBuffer = PxStrideIterator<const PxVec3>(positions);
    particleCreationData.velocityBuffer = PxStrideIterator<const PxVec3>(velocities, 0);

    bool success = m_particleSystem->createParticles(particleCreationData);

    if (success)
        m_particleDrawable->addParticles(numParticles, positions);
    else
        glow::warning("ParticleGroup::createParticles creation of %; physx particles failed", numParticles);
}

void ParticleGroup::updateVisuals()
{    
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    m_particleDrawable->updateParticles(readData);

    readData->unlock();
}

void ParticleGroup::setImmutableProperties(const ImmutableParticleProperties & properties)
{
    setImmutableProperties(properties.maxMotionDistance, properties.gridSize, properties.restOffset, properties.contactOffset, properties.restParticleDistance);
}

void ParticleGroup::setMutableProperties(const MutableParticleProperties & properties)
{
    setMutableProperties(properties.restitution, properties.dynamicFriction, properties.staticFriction, properties.damping, /*properties.externalAcceleration,*/ properties.particleMass, properties.viscosity, properties.stiffness);
}

void ParticleGroup::setImmutableProperties(const physx::PxReal maxMotionDistance, const physx::PxReal gridSize, const physx::PxReal restOffset, const physx::PxReal contactOffset, const physx::PxReal restParticleDistance)
{
    assert(m_particleSystem);
    assert(m_scene);

    m_scene->removeActor(*m_particleSystem);

    m_particleSystem->setMaxMotionDistance(maxMotionDistance);
    m_particleSystem->setGridSize(gridSize);
    m_particleSystem->setRestOffset(restOffset);
    m_particleSystem->setContactOffset(contactOffset);
    m_particleSystem->setRestParticleDistance(restParticleDistance);

    m_scene->addActor(*m_particleSystem);
}

void ParticleGroup::setMutableProperties(const physx::PxReal restitution, const physx::PxReal dynamicFriction, const physx::PxReal staticFriction, const physx::PxReal damping, /*const physx::PxVec3 externalAcceleration,*/ const physx::PxReal particleMass, const physx::PxReal viscosity, const physx::PxReal stiffness)
{
    m_particleSystem->setRestitution(restitution);
    m_particleSystem->setDynamicFriction(dynamicFriction);
    m_particleSystem->setStaticFriction(staticFriction);
    m_particleSystem->setDamping(damping);
    // m_particleSystem->setExternalAcceleration(externalAcceleration);
    m_particleSystem->setParticleMass(particleMass);
    m_particleSystem->setViscosity(viscosity);
    m_particleSystem->setStiffness(stiffness);
}
