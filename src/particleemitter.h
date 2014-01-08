#pragma once

#include <memory>

#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>

namespace physx {
    class PxParticleSystem;
}
class ParticleDrawable;

typedef long double t_longf;

class ParticleEmitter
{
public:
    ParticleEmitter(const physx::PxVec3& position = physx::PxVec3(0, 0, 0));
    ~ParticleEmitter();

    void initializeParticleSystem();
    void update(t_longf elapsed_Time);

    void startEmit();
    void stopEmit();

    void createParticles(int number_of_particles);

protected:
    static const physx::PxU32	kMaxParticleCount;
    static const physx::PxU32   kDefaultEmittedParticles;

    std::shared_ptr<ParticleDrawable> m_particleDrawable;

    physx::PxVec3            m_position;
    bool                     m_emitting;
    int                      m_particles_per_second;

    t_longf                  akkumulator;
    size_t                   youngest_particle_index;


    physx::PxParticleSystem*            m_particleSystem; // or fluid?

public:
    ParticleEmitter(ParticleEmitter&) = delete;
    void operator=(ParticleEmitter&) = delete;
};