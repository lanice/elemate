#pragma once


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
        const ImmutableParticleProperties & immutProperties,
        const MutableParticleProperties & mutProperties
        );
    ~ParticleGroup();

    void createParticles(const physx::PxU32 number, const physx::PxVec3 & position);
    void startEmit(const physx::PxU32 particleRate, const physx::PxVec3 & position);
    void stopEmit();

    void update();

    void setImmutableProperties(const ImmutableParticleProperties & properties);
    void setMutableProperties(const MutableParticleProperties & properties);


private:
    ImmutableParticleProperties m_immutProperties;
    MutableParticleProperties m_mutProperties;

public:
    ParticleGroup(ParticleGroup&) = delete;
    void operator=(ParticleGroup&) = delete;
};
