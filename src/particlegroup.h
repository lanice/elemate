#pragma once

#include <memory>

#include "pxcompilerfix.h"
#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>


namespace physx {
    class PxParticleFluid;
    class PxScene;
}
class ParticleDrawable;

struct ImmutableParticleProperties
{
    physx::PxReal maxMotionDistance = 1.0f;
    physx::PxReal gridSize = 0.0f;
    physx::PxReal restOffset = 0.3f;
    physx::PxReal contactOffset = 0.3f;

    physx::PxReal restParticleDistance = 0.3f;
};

struct MutableParticleProperties
{
    physx::PxReal restitution = 0.5f;
    physx::PxReal dynamicFriction = 0.5f;
    physx::PxReal staticFriction = 0.5f;
    physx::PxReal damping = 0.5f;
    physx::PxVec3 externalAcceleration = physx::PxVec3(0.0f, 0.0f, 0.0f);
    physx::PxReal particleMass = 0.5f;

    physx::PxReal viscosity = 5.0f;
    physx::PxReal stiffness = 8.134f;
};

class ParticleGroup
{
public:
    ParticleGroup(
        const physx::PxU32 maxParticleCount,
        const ImmutableParticleProperties & immutableProperties,
        const MutableParticleProperties & mutableProperties
        );
    ~ParticleGroup();

    void createParticles(const physx::PxU32 numParticles, const physx::PxU32 * indices, const physx::PxVec3 * positions, const physx::PxVec3 * velocities);
    void startEmit(const physx::PxU32 particleRate, const physx::PxVec3 & position);
    void stopEmit();

    void updateVisuals();

    void setImmutableProperties(const ImmutableParticleProperties & properties);
    void setMutableProperties(const MutableParticleProperties & properties);


protected:
    physx::PxParticleFluid * m_particleSystem;
    physx::PxScene * m_scene;

    std::shared_ptr<ParticleDrawable> m_particleDrawable;

    ImmutableParticleProperties m_immutableProperties;
    MutableParticleProperties m_mutableProperties;


public:
    ParticleGroup(ParticleGroup&) = delete;
    void operator=(ParticleGroup&) = delete;
};
