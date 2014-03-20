#pragma once

#include <memory>
#include <vector>

#include "utils/pxcompilerfix.h"
#include <foundation/PxSimpleTypes.h>

#include <glm/glm.hpp>

namespace physx {
    class PxParticleFluid;
    class PxScene;
    class PxParticleReadData;
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
    glm::vec3 externalAcceleration;
    physx::PxReal particleMass = 0.001f;

    physx::PxReal viscosity = 5.0f;
    physx::PxReal stiffness = 8.134f;
};

class ParticleGroup
{
public:
    ParticleGroup(
        const std::string & elementName,
        const unsigned int id,
        const bool enableGpuParticles,
        const bool isDown,
        const uint32_t maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );
    virtual ~ParticleGroup();

    /** copy all attributes of the particle group (but not the particles) */
    ParticleGroup(const ParticleGroup & lhs, unsigned int id);

    const std::string & elementName() const;
    uint32_t numParticles() const;
    const glowutils::AxisAlignedBoundingBox & boundingBox() const;
    float particleSize() const;
    void setParticleSize(float size);
    const bool isDown;

    float temperature() const;
    void setTemperature(float temperature);

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
    void releaseParticlesGetPositions(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & releasedPositions, glowutils::AxisAlignedBoundingBox & releasedBounds);

    /** fill particles with all my particles which have there center in the specified bounding box.
      * @param subbox is the axis aligned bounding box of the particles that are inside the input bounding box.
      * This should only be called while the physics scene simulation is not running! */
    void particlesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & particles, glowutils::AxisAlignedBoundingBox & subbox) const;
    /** Get the indexes of the particles that are inside the bounding box.
      * This doesn't clear the particleIndicies container, if it contained any elements before. */
    void particleIndicesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<uint32_t> & particleIndices) const;
    void particlePositionsIndicesVelocitiesInVolume(const glowutils::AxisAlignedBoundingBox & boundingBox, std::vector<glm::vec3> & positions, std::vector<uint32_t> & particleIndices, std::vector<glm::vec3> & velocities) const;

    virtual void updatePhysics(double delta);
    /** Subscribed to World to update particle visuals. */
    virtual void updateVisuals();

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
        const glm::vec3 &externalAcceleration,
        const physx::PxReal particleMass,
        const physx::PxReal viscosity,
        const physx::PxReal stiffness
        );

    void setUseGpuParticles(const bool enable);
    bool useGpuParticles() const;

    void giveGiftTo(ParticleGroup & other);
    void updateSounds(bool isWorldPaused);
    void startSound();
    void stopSound();
protected:
    void releaseOldParticles(const uint32_t numParticles);

    const unsigned int m_id;

    ImmutableParticleProperties m_immutableProperties;
    MutableParticleProperties m_mutableProperties;
    physx::PxParticleFluid * m_particleSystem;
    physx::PxScene * m_scene;

    std::string m_elementName;

    float m_particleSize;
    float m_temperature;

    std::shared_ptr<ParticleDrawable> m_particleDrawable;

    const uint32_t m_maxParticleCount;
    uint32_t m_numParticles;
    physx::PxU32 * m_indices;
    std::vector<physx::PxU32> m_freeIndices;
    uint32_t m_nextFreeIndex;
    uint32_t m_lastFreeIndex;

    bool m_gpuParticles;

    bool m_wasSoundPlaying;
    unsigned int m_soundChannel;
    std::vector<uint32_t> m_particlesToDelete;

public:
    void operator=(ParticleGroup&) = delete;
};
