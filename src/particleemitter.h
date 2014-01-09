#pragma once

#include <memory>

#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>

namespace physx {
    class PxParticleFluid;
}
class ParticleDrawable;

class ParticleEmitter
{
public:
    ParticleEmitter(const physx::PxVec3& position = physx::PxVec3(0, 0, 0));
    ~ParticleEmitter();

    void initializeParticleSystem();
    void update(double elapsed_Time);

    void startEmit();
    void stopEmit();

    void setPosition(const physx::PxVec3& position);
    physx::PxVec3 position()const;

    //void setEmittingRate(const physx::PxU32& particle_per_emit);
    //void setMaxParticles(const physx::PxU32& particle_count);

    void createParticles(physx::PxU32 number_of_particles);

protected:
    static const physx::PxU32	kMaxParticleCount;
    static const physx::PxU32   kDefaultEmittedParticles;

    std::shared_ptr<ParticleDrawable> m_particleDrawable;

    physx::PxVec3            m_position;
    bool                     m_emitting;
    int                      m_particles_per_second;

    float                    m_akkumulator;
    physx::PxU32             m_youngest_particle_index;
    bool                     m_reuses_old_particle;
    physx::PxU32             m_reuse_limitation;


    physx::PxParticleFluid*  m_particleSystem; // or fluid?

public:
    ParticleEmitter(ParticleEmitter&) = delete;
    void operator=(ParticleEmitter&) = delete;
};