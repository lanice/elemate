#pragma once

#include <memory>
#include <list>

#include "pxcompilerfix.h"
#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>

namespace physx{
    class PxParticleBase;
    class PxParticleCreationData;
}

namespace physx {
    class PxParticleCreationData;
}

class ParticleManagement
{
public:
    ParticleManagement(physx::PxParticleBase* particleBase);
    ~ParticleManagement();

    std::shared_ptr<physx::PxParticleCreationData> particleCreationData(physx::PxU32 number_of_particles, physx::PxVec3 position);
    std::shared_ptr<std::list<physx::PxVec3>> restingParticles();
    std::shared_ptr<std::list<physx::PxVec3>> visibleParticles();

protected:
    static const size_t kMaxParticleCount = 10000;
    static const physx::PxU32   kDefaultEmittedParticles;
    static const physx::PxReal  kDefaultInitialParticleSpeed;
    static const int            kDefaultParticleSpreading;

    std::shared_ptr<std::list<physx::PxVec3>> m_restingParticles;
    std::shared_ptr<std::list<physx::PxVec3>> m_visibleParticles;

    physx::PxU32    m_particle_index_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_position_buffer[kMaxParticleCount];
    physx::PxVec3   m_particle_velocity_buffer[kMaxParticleCount];

    unsigned int m_youngest_particle_index;
    unsigned int m_reuse_limitation;
    bool m_reuses_old_particle;
    physx::PxParticleBase* m_particleBase;
public:
    ParticleManagement(ParticleManagement&) = delete;
    void operator=(ParticleManagement&) = delete;
};
