#pragma once

#include <memory>
#include <vector>

#include "utils/pxcompilerfix.h"
#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>

#include <glm/glm.hpp>

namespace physx {
    class PxParticleFluid;
    class PxScene;
}
namespace glowutils { class AxisAlignedBoundingBox; }
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
        const std::string & elementName,
        const bool enableGpuParticles,
        const uint32_t maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );
    ~ParticleGroup();

    const std::string & elementName() const;
    const glowutils::AxisAlignedBoundingBox & boundingBox() const;
    float particleSize() const;
    void setParticleSize(float size);

    physx::PxParticleFluid * particleSystem();

    /** If specifying velocities, make sure that its size matches the positions size */
    void createParticles(const std::vector<glm::vec3> & positions, const std::vector<glm::vec3> * velocities = nullptr);
    /** Create a single particle at given position with given velocity. */
    void createParticle(const glm::vec3 & position, const glm::vec3 & velocity);
    void releaseParticles(const std::vector<uint32_t> & indices);
    /** release all particles that are inside the bounding box
      * @return the number of particles that was deleted. */
    uint32_t releaseParticles(const glowutils::AxisAlignedBoundingBox & boundingBox);
    /** Release particles that are inside the bounding box and append their positions to the releasedPositions vector. */
    void releaseParticlesGetPositions(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & releasedPositions);

    /** Emit particles with ratio as particles per second. */
    void emit(const float ratio, const glm::vec3 & position, const glm::vec3 & direction);
    void stopEmit();

    /** fill particles with all my particles which have there center in the specified bounding box.
      * @param subbox is the axis aligned bounding box of the particles that are inside the input bounding box.
      * This should only be called while the physics scene simulation is not running! */
    void particlesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & particles, glowutils::AxisAlignedBoundingBox & subbox) const;
    /** Get the indexes of the particles that are inside the bounding box.
      * This doesn't clear the particleIndicies container, if it contained any elements before. */
    void particleIndicesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<uint32_t> & particleIndices) const;
    /** Get the positions and indexes of the particles that are inside the bounding box.
      * This doesn't clear the referenced containers, if they contained any elements before. */
    void particlePositionsIndicesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & positions, std::vector<uint32_t> & particleIndices) const;

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
    void releaseOldParticles(const uint32_t numParticles);

    physx::PxParticleFluid * m_particleSystem;
    physx::PxScene * m_scene;

    const std::string m_elementName;

    float m_particleSize;

    std::shared_ptr<ParticleDrawable> m_particleDrawable;

    uint32_t m_maxParticleCount;
    physx::PxU32 * m_indices;
    std::vector<physx::PxU32> m_freeIndices;
    uint32_t m_nextFreeIndex;
    uint32_t m_lastFreeIndex;

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
