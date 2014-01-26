#pragma once

#include <memory>
#include <vector>

#include <glow/Array.h>

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

    physx::PxReal restParticleDistance = 0.1f;
};

struct MutableParticleProperties
{
    physx::PxReal restitution = 0.5f;
    physx::PxReal dynamicFriction = 0.05f;
    physx::PxReal staticFriction = 0.0f;
    physx::PxReal damping = 0.0f;
    physx::PxVec3 externalAcceleration = physx::PxVec3(0.0f, 0.0f, 0.0f);
    physx::PxReal particleMass = 0.001f;

    physx::PxReal viscosity = 5.0f;
    physx::PxReal stiffness = 8.134f;
};

class ParticleGroup
{
public:
    ParticleGroup(
        const bool enableGpuParticles,
        const uint32_t maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );
    ~ParticleGroup();

    /** Make sure numParticles matches size of position matches size of velocities! */
    void createParticles(const uint32_t numParticles, const glow::Vec3Array & positions, const glow::Vec3Array & velocities);
    /** Create a single particle at given position with given velocity. */
    void createParticle(const glm::vec3 & position, const glm::vec3 & velocity);

    /** Emit particles with ratio as particles per second. */
    void emit(float ratio, const glm::vec3 & position, const glm::vec3 & direction);
    void stopEmit();

    /** Subscribed to World to receive time delta for timed emit of particles. (Observer pattern) */
    void updateEmitting(const double & delta);
    /** Subscribed to World to update particle visuals. (Observer pattern) */
    void updateVisuals();

    void setImmutableProperties(const ImmutableParticleProperties & properties);
    void setMutableProperties(const MutableParticleProperties & properties);

    void setImmutableProperties(
        const physx::PxReal maxMotionDistance,
        const physx::PxReal gridSize,
        const physx::PxReal restOffset,
        const physx::PxReal contactOffset,
        const physx::PxReal restParticleDistance
        );
    void setMutableProperties(
        const physx::PxReal restitution,
        const physx::PxReal dynamicFriction,
        const physx::PxReal staticFriction,
        const physx::PxReal damping,
        // const physx::PxVec3 externalAcceleration,
        const physx::PxReal particleMass,
        const physx::PxReal viscosity,
        const physx::PxReal stiffness
        );

    void setUseGpuParticles(const bool enable);


protected:
    physx::PxParticleFluid * m_particleSystem;
    physx::PxScene * m_scene;

    std::shared_ptr<ParticleDrawable> m_particleDrawable;

    uint32_t m_maxParticleCount;
    physx::PxU32 * m_indices;
    std::vector<physx::PxU32> m_freeIndices;
    uint32_t m_nextFreeIndex;

    float m_emitRatio;
    glm::vec3 m_emitPosition;
    glm::vec3 m_emitDirection;
    bool m_emitting;
    double m_timeSinceLastEmit;

    bool m_gpuParticles;


public:
    ParticleGroup(ParticleGroup&) = delete;
    void operator=(ParticleGroup&) = delete;
};
