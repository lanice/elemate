#include "particlegroup.h"

#include <cassert>
#include <random>
#include <ctime>
#include <functional>

#include <glow/logging.h>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxSceneLock.h>

#include "particledrawable.h"


namespace {
    std::mt19937 rng;
}

using namespace physx;

namespace {
    uint32_t seed_val = static_cast<uint32_t>(std::time(0));
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

ParticleGroup::ParticleGroup(
    const bool enableGpuParticles,
    const PxU32 maxParticleCount,
    const ImmutableParticleProperties & immutableProperties,
    const MutableParticleProperties & mutableProperties
    )
: m_particleSystem(nullptr)
, m_scene(nullptr)
, m_particleDrawable(std::make_shared<ParticleDrawable>(maxParticleCount))
, m_maxParticleCount(maxParticleCount)
, m_indices(new PxU32[maxParticleCount]())
, m_nextFreeIndex(0)
, m_lastFreeIndex(maxParticleCount-1)
, m_emitting(false)
, m_timeSinceLastEmit(0.0)
, m_gpuParticles(enableGpuParticles)
{
    for (PxU32 i = 0; i < maxParticleCount; ++i) m_indices[i] = i;

    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    m_scene = pxScenePtrs[0];

    PxSceneWriteLock scopedLock(* m_scene);

    m_particleSystem = PxGetPhysics().createParticleFluid(maxParticleCount, false);
    assert(m_particleSystem);
    m_particleSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, m_gpuParticles);

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

physx::PxParticleFluid * ParticleGroup::particleSystem()
{
    return m_particleSystem;
}

physx::PxScene * ParticleGroup::physxScene()
{
    return m_scene;
}

void ParticleGroup::createParticles(const uint32_t numParticles, const glow::Vec3Array & pos, const glow::Vec3Array & vel)
{
    PxU32 * indices = new PxU32[numParticles];
    PxVec3 * positions = new PxVec3[numParticles];
    PxVec3 * velocities = new PxVec3[numParticles];

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        if (m_freeIndices.size() > 0)
        {
            indices[i] = m_freeIndices.back();
            m_freeIndices.pop_back();
        } else {
            indices[i] = m_nextFreeIndex;
            if (m_nextFreeIndex == m_lastFreeIndex) releaseOldParticles(numParticles);
            if (++m_nextFreeIndex == m_maxParticleCount) m_nextFreeIndex = 0;
        }

        positions[i] = PxVec3(pos.at(i).x, pos.at(i).y, pos.at(i).z);
        velocities[i] = PxVec3(vel.at(i).x, vel.at(i).y, vel.at(i).z);
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

    delete[] indices;
    delete[] positions;
    delete[] velocities;
}

void ParticleGroup::releaseOldParticles(const uint32_t numParticles)
{
    glow::UIntArray indices;
    for (uint32_t i = 0; i < numParticles; ++i)
    {
        if (++m_lastFreeIndex == m_maxParticleCount) m_lastFreeIndex = 0;
        indices << m_lastFreeIndex;
    }

    PxU32 * indexBuffer = new PxU32[numParticles];

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        PxU32 index = indices.at(i);
        indexBuffer[i] = index;
    }

    PxStrideIterator<const PxU32> indexxBuffer(indexBuffer);

    m_particleSystem->releaseParticles(numParticles, indexxBuffer);

    delete[] indexBuffer;
}

void ParticleGroup::releaseParticles(const uint32_t numParticles, const glow::UIntArray & indices)
{
    PxU32 * indexBuffer = new PxU32[numParticles];

    for (PxU32 i = 0; i < numParticles; ++i)
    {
        PxU32 index = indices.at(i);
        indexBuffer[i] = index;
        m_freeIndices.push_back(index);
    }

    PxStrideIterator<const PxU32> indexxBuffer(indexBuffer);

    m_particleSystem->releaseParticles(numParticles, indexxBuffer);

    delete[] indexBuffer;
}

void ParticleGroup::createParticle(const glm::vec3 & position, const glm::vec3 & velocity)
{
    glow::Vec3Array positions, velocities;
    positions << position;
    velocities << velocity;
    createParticles(1, positions, velocities);
}

void ParticleGroup::emit(const float ratio, const glm::vec3 & position, const glm::vec3 & direction)
{
    m_emitRatio = ratio;
    m_emitPosition = position;
    m_emitDirection = glm::normalize(direction);
    m_emitting = true;
}

void ParticleGroup::stopEmit()
{
    m_emitting = false;
    m_timeSinceLastEmit = 0.0;
}

void ParticleGroup::updateEmitting(const double & delta)
{
    if (!m_emitting) return;

    unsigned int particlesToEmit = glm::floor(m_emitRatio * delta);

    std::uniform_real_distribution<float> uniform_dist(-0.75f, 0.75f);
    std::function<float()> scatterFactor = [&](){ return uniform_dist(rng); };

    if (particlesToEmit > 0)
    {
        for (unsigned int i = 0; i < particlesToEmit; ++i)
            createParticle(m_emitPosition, glm::vec3((m_emitDirection.x + scatterFactor()), (m_emitDirection.y + scatterFactor()), (m_emitDirection.z + scatterFactor())) * 100.f);

        m_timeSinceLastEmit = 0.0;
    } else {
        if (m_timeSinceLastEmit >= 1.0 / m_emitRatio)
        {
            createParticle(m_emitPosition, glm::vec3((m_emitDirection.x + scatterFactor()), (m_emitDirection.y + scatterFactor()), (m_emitDirection.z + scatterFactor())) * 100.f);
            m_timeSinceLastEmit = 0.0;
        } else {
            m_timeSinceLastEmit += delta;
        }
    }
}

void ParticleGroup::updateVisuals()
{    
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    m_particleDrawable->updateParticles(readData);

    // Get drained Particles
    glow::UIntArray indices;
    PxStrideIterator<const PxParticleFlags> flagsIt(readData->flagsBuffer);

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++flagsIt)
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_DRAIN)
            indices << i;

    readData->unlock();

    m_particleDrawable->setParticleSize(m_particleSystem->getRestParticleDistance());

    releaseParticles(indices.size(), indices);
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

    PxSceneWriteLock scopedLock(* m_scene);

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

void ParticleGroup::setUseGpuParticles(const bool enable)
{
    if (m_gpuParticles == enable) return;

    m_gpuParticles = enable;

    assert(m_particleSystem);
    assert(m_scene);

    m_scene->removeActor(*m_particleSystem);

    m_particleSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, m_gpuParticles);

    m_scene->addActor(*m_particleSystem);
}
