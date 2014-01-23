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
    physx::PxReal maxMotionDistance;
    physx::PxReal gridSize;
    physx::PxReal restOffset;
    physx::PxReal contactOffset;

    physx::PxReal restParticleDistance;
};

struct MutableParticleProperties
{
    physx::PxReal restitution;
    physx::PxReal dynamicFriction;
    physx::PxReal staticFriction;
    physx::PxReal damping;
    physx::PxVec3 externalAcceleration;
    physx::PxReal particleMass;

    physx::PxReal viscosity;
    physx::PxReal stiffness;
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
