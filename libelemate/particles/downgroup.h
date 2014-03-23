#pragma once

#include "particlegroup.h"

/** @brief ParticleGroup that contains particles which collided with the ground/water at least once. */
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

    /** copy all attributes of the particle group (but not the particles) */
    DownGroup(const ParticleGroup& lhs, unsigned int id);
    /** copy all attributes of the particle group (but not the particles) */
    DownGroup(const DownGroup& lhs, unsigned int id);

    /** Update physics of contained particles. */
    virtual void updatePhysics(double delta) override;

    /** Update visuals of contained particles. */
    virtual void updateVisuals() override;

public:
    void operator=(ParticleGroup&) = delete;
};
