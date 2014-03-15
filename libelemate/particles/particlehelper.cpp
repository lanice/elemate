#include "particlehelper.h"

#include <glowutils/AxisAlignedBoundingBox.h>


uint8_t longestAxis(const glowutils::AxisAlignedBoundingBox & bounds, float & splitValue)
{
    glm::vec3 dimensions = bounds.urb() - bounds.llf();

    if (dimensions.x > dimensions.y) {
        if (dimensions.x > dimensions.z) { // x-split
            splitValue = (bounds.urb().x - bounds.llf().x) * 0.5f + bounds.llf().x;
            return 0;
        }
    }
    else {
        if (dimensions.y > dimensions.z) { // y-split
            splitValue = (bounds.urb().y - bounds.llf().y) * 0.5f + bounds.llf().y;
            return 1;
        }
    }
    // z-split
    splitValue = (bounds.urb().z - bounds.llf().z) * 0.5f + bounds.llf().z;
    return 2;
}