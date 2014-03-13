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

protected:
    virtual void updateVisualsAmpl(const physx::PxParticleReadData & readData) override;

public:
    EmitterGroup() = delete;
    void operator=(EmitterGroup &) = delete;
};
