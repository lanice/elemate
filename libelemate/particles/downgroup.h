#pragma once

#include "particlegroup.h"

class DownGroup : public ParticleGroup
{
public:
    DownGroup(
        const std::string & elementName,
        const bool enableGpuParticles,
        const uint32_t maxParticleCount = 10000,
        const ImmutableParticleProperties & immutableProperties = ImmutableParticleProperties(),
        const MutableParticleProperties & mutableProperties = MutableParticleProperties()
        );

protected:
    virtual void updateVisuals() override;


public:
    DownGroup() = delete;
    void operator=(DownGroup &) = delete;
};
