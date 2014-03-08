#pragma once

#include "particlegroup.h"


class EmitterGroup : public ParticleGroup
{
public:
    EmitterGroup(
        const std::string & elementName,
        const bool enableGpuParticles,
        const uint32_t maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );

    virtual void updateVisuals() override;

protected:
    std::vector<glm::vec3> m_downPositions;
    std::vector<glm::vec3> m_downVelocities;

public:
    EmitterGroup() = delete;
    void operator=(EmitterGroup &) = delete;
};
