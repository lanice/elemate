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
    physx::PxReal maxMotionDistance = 0.06f;
    physx::PxReal gridSize = 0.64f;
    physx::PxReal restOffset = 0.004f;
    physx::PxReal contactOffset = 0.008f;

    physx::PxReal restParticleDistance = 0.03f;
};

struct MutableParticleProperties
{
    physx::PxReal restitution = 0.5f;
    physx::PxReal dynamicFriction = 0.05f;
    physx::PxReal staticFriction = 0.0f;
    physx::PxReal damping = 0.0f;
    physx::PxVec3 externalAcceleration = physx::PxVec3(0.0f, 0.0f, 0.0f);
    physx::PxReal particleMass = 0.001f;

    physx::PxReal viscosity = 0.5f;
    physx::PxReal stiffness = 8.134f;
};

class ParticleGroup
{
public:
    ParticleGroup(
        const physx::PxU32 maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );
    ~ParticleGroup();

    void createParticles(const physx::PxU32 numParticles, const physx::PxU32 * indices, const physx::PxVec3 * positions, const physx::PxVec3 * velocities);

    void updateVisuals();

    void setImmutableProperties(const ImmutableParticleProperties & properties);
    void setMutableProperties(const MutableParticleProperties & properties);

    int setImmutableProperties(
        const physx::PxReal maxMotionDistance,
        const physx::PxReal gridSize,
        const physx::PxReal restOffset,
        const physx::PxReal contactOffset,
        const physx::PxReal restParticleDistance
        );
    int setMutableProperties(
        const physx::PxReal restitution,
        const physx::PxReal dynamicFriction,
        const physx::PxReal staticFriction,
        const physx::PxReal damping,
        // const physx::PxVec3 externalAcceleration,
        const physx::PxReal particleMass,
        const physx::PxReal viscosity,
        const physx::PxReal stiffness
        );


protected:
    physx::PxParticleFluid * m_particleSystem;
    physx::PxScene * m_scene;

    std::shared_ptr<ParticleDrawable> m_particleDrawable;


public:
    ParticleGroup(ParticleGroup&) = delete;
    void operator=(ParticleGroup&) = delete;
};
