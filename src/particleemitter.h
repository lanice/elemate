#pragma once

#include <memory>

#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>

namespace physx {
    class PxParticleFluid;
    struct PxFilterData;
    class PxVec3;
}
class ParticleDrawable;

class EmitterDescriptionData{
public:
// immutable properties
    //PxParticleBase (Fluid and System)
    
    physx::PxReal maxMotionDistance = 1.0F; // The maximum distance a particle can travel during one simulation step.High values may hurt performance, while low values may restrict the particle velocity too much.
    physx::PxReal gridSize = 0.0F;          // A hint for the PhysX SDK to choose the particle grouping granularity for proximity tests and parallelization.See particleGrid.
    physx::PxReal restOffset = 0.3F;        // Defines the minimum distance between particles and the surface of rigid actors that is maintained by the collision system.
    physx::PxReal contactOffset = 0.3F;     // Defines the distance at which contacts between particles and rigid actors are created.The contacts are internally used to avoid jitter and sticking.It needs to be larger than restOffset.
    
    //PxParticleFluids only                        
    physx::PxReal restParticleDistance = 0.3F; // Defines the resolution of the particle fluid.

// mutable properties
    //PxParticleBase (Fluid and System)
    physx::PxReal restitution           = 0.5F; //Restitution used for particle collision with shapes between 0 and 1.
    physx::PxReal dynamicFriction       = 0.5F; // Dynamic friction used for particle collision.
    physx::PxReal staticFriction        = 0.5F; //Static friction used for particle collision.
    physx::PxReal damping               = 0.5F; //Velocity damping constant, which is globally applied to each particle.
    physx::PxVec3 externalAcceleration  = physx:: PxVec3(0.0F, 0.0F, 0.0F); //Acceleration applied to each particle at each time step.The scene gravity which is added to the external acceleration by default can be disabled using PxActorFlag::eDISABLE_GRAVITY.
    physx::PxReal particleMass          = 0.5F; //Mass used for two way interaction with rigid bodies.
    physx::PxFilterData* simulationFilterData  = nullptr; //Filter data used to filter collisions between particles and rigid bodies.See collisionFiltering.

    //PxParticleFluid only
    physx::PxReal viscosity = 5.0F; //From Water to goo ... between 5 and 300 is reasonable
    physx::PxReal stiffness = 8.134F; //Gas constant
};

class ParticleEmitter
{
public:
    ParticleEmitter(bool gpuParticles, const physx::PxVec3& position = physx::PxVec3(0, 0, 0));
    ~ParticleEmitter();

    void initializeParticleSystem(EmitterDescriptionData* descriptionData);
    /** creates particles when enough time is accumulated */
    void step(double elapsed_Time);
    /** fetch the particle positions and copies it to the drawables */
    void update();

    void startEmit();
    void stopEmit();

    void setPosition(const physx::PxVec3& position);
    physx::PxVec3 position()const;
    void setEmittingRate(const physx::PxU32& particle_per_emit);

    void createParticles(physx::PxU32 number_of_particles);

    /** enable/disable GPU acceleration. Will cause errors if not running on nvidia gpu. */
    void setGPUAccelerated(bool enable);

protected:
    static const physx::PxU32	kMaxParticleCount;
    static const physx::PxU32   kDefaultEmittedParticles;
    static const physx::PxReal  kDefaultInitialParticleSpeed;
    static const int            kDefaultParticleSpreading;

    void applyDescriptionData(EmitterDescriptionData* descriptionData);

    std::shared_ptr<ParticleDrawable> m_particleDrawable;

    bool                     m_gpuParticles;
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