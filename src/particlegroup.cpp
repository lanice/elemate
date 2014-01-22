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
, m_immutableProperties(immutableProperties)
, m_mutableProperties(mutableProperties)
{
    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    m_scene = pxScenePtrs[0];

    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem = PxGetPhysics().createParticleFluid(maxParticleCount, false);
    assert(m_particleSystem);
    // m_particleSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, m_gpuParticles);
    setImmutableProperties(immutableProperties);
    setMutableProperties(mutableProperties);

    m_scene->addActor(*m_particleSystem);
}

ParticleGroup::~ParticleGroup()
{
    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem->releaseParticles();
    m_scene->removeActor(*m_particleSystem);
    m_particleSystem = nullptr;
}

void ParticleGroup::createParticles(const physx::PxU32 numParticles, const physx::PxU32 * indices, const physx::PxVec3 * positions, const physx::PxVec3 * velocities)
{
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
    // m_particleSystem->setMaxMotionDistance(properties.maxMotionDistance);
    // m_particleSystem->setGridSize(properties.gridSize);
    // m_particleSystem->setRestOffset(properties.restOffset);
    // m_particleSystem->setContactOffset(properties.contactOffset);
    m_particleSystem->setRestParticleDistance(properties.restParticleDistance);
}

void ParticleGroup::setMutableProperties(const MutableParticleProperties & properties)
{
    // m_particleSystem->setRestitution(properties.restitution);
    // m_particleSystem->setDynamicFriction(properties.dynamicFriction);
    // m_particleSystem->setStaticFriction(properties.staticFriction);
    // m_particleSystem->setExternalAcceleration(properties.externalAcceleration);
    // m_particleSystem->setParticleMass(properties.particleMass);
    m_particleSystem->setViscosity(properties.viscosity);
    m_particleSystem->setStiffness(properties.stiffness);
}
