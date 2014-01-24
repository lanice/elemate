#include "particlemanagement.h"

#include <cassert>

#include <glow/logging.h>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxSceneLock.h>

// #include "PxPhysicsAPI.h"

#include "particledrawable.h"

const physx::PxU32  ParticleManagement::kDefaultEmittedParticles = 1;
const physx::PxReal ParticleManagement::kDefaultInitialParticleSpeed = 0.5F;
const int           ParticleManagement::kDefaultParticleSpreading = 50;

ParticleManagement::ParticleManagement(physx::PxParticleBase* particleBase)
: m_youngest_particle_index(0)
, m_reuses_old_particle(false)
, m_reuse_limitation(0)
, m_particleBase(particleBase)
{

}

ParticleManagement::~ParticleManagement()
{

}

std::shared_ptr<physx::PxParticleCreationData> ParticleManagement::particleCreationData(physx::PxU32 number_of_particles, physx::PxVec3 position)
{
    if (number_of_particles > kMaxParticleCount)
        number_of_particles = kMaxParticleCount;

    number_of_particles = number_of_particles + m_youngest_particle_index > kMaxParticleCount ? kMaxParticleCount - m_youngest_particle_index : number_of_particles;

    m_reuse_limitation = m_youngest_particle_index;

    auto particleCreationData =std::make_shared<physx::PxParticleCreationData>();
    particleCreationData->numParticles = number_of_particles;

    for (physx::PxU32 i = 0; i < number_of_particles; i++)
    {
        m_particle_index_buffer[i] = (m_youngest_particle_index + i) % kMaxParticleCount;
        m_particle_position_buffer[i] = position;
        m_particle_velocity_buffer[i] = physx::PxVec3(0.0F + (rand() % kDefaultParticleSpreading - (kDefaultParticleSpreading / 2)),
            -1.0F,
            0.0F + (rand() % kDefaultParticleSpreading - (kDefaultParticleSpreading / 2)))
            .getNormalized()*kDefaultInitialParticleSpeed;
    }
    m_youngest_particle_index = (m_youngest_particle_index + number_of_particles) % kMaxParticleCount;

    particleCreationData->indexBuffer = physx::PxStrideIterator<const physx::PxU32>(m_particle_index_buffer);
    particleCreationData->positionBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_position_buffer);
    particleCreationData->velocityBuffer = physx::PxStrideIterator<const physx::PxVec3>(m_particle_velocity_buffer);

    if (m_reuses_old_particle)
        m_particleBase->releaseParticles(number_of_particles, particleCreationData->indexBuffer);
    m_reuses_old_particle |= m_reuse_limitation > m_youngest_particle_index;

    return particleCreationData;
}
std::shared_ptr<std::list<physx::PxVec3>> ParticleManagement::restingParticles()
{
    return nullptr;
}

std::shared_ptr<std::list<physx::PxVec3>> ParticleManagement::visibleParticles()
{
    return nullptr;
}
