#pragma once

#include "particlegroup.h"

class DownGroup : public ParticleGroup
{
public:
    DownGroup(
        const std::string & elementName,
        const unsigned int id,
        const bool enableGpuParticles,
        const uint32_t maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );

    DownGroup(const ParticleGroup& lhs, unsigned int id);
    DownGroup(const DownGroup& lhs, unsigned int id);

protected:
    virtual void updatePhysics(double delta) override;
    virtual void updateVisuals() override;

public:
    void operator=(ParticleGroup&) = delete;
};
