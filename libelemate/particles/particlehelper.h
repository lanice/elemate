#pragma once

#include <cstdint>

namespace glowutils
{
    class AxisAlignedBoundingBox;
}

uint8_t longestAxis(const glowutils::AxisAlignedBoundingBox & bounds, float & splitValue);

