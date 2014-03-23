#pragma once

#include <cstdint>

namespace glowutils
{
    class AxisAlignedBoundingBox;
}

/** Returns longest axis of a given bounding box and stores the value at which position of the axis the ParticleGroup should be splitted into splitValue. */
uint8_t longestAxis(const glowutils::AxisAlignedBoundingBox & bounds, float & splitValue);

