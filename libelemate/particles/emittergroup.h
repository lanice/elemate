#pragma once

#include "particlegroup.h"


class EmitterGroup : public ParticleGroup
{
public:
    EmitterGroup(
        const std::string & elementName,
        const unsigned int id,
        const bool enableGpuParticles,
        const uint32_t maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );

    /** Emit particles with ratio as particles per second. */
    void emit(const float ratio, const glm::vec3 & position, const glm::vec3 & direction);
    void stopEmit();

    virtual void updatePhysics(double delta) override;
    virtual void updateVisuals() override;

protected:
    std::vector<glm::vec3> m_downPositions;
    std::vector<glm::vec3> m_downVelocities;

    float m_emitRatio;
    glm::vec3 m_emitPosition;
    glm::vec3 m_emitDirection;
    double m_timeSinceLastEmit;
    bool m_emitting;

public:
    EmitterGroup() = delete;
    void operator=(EmitterGroup &) = delete;
};
