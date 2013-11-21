#include "xzPlaneLocator.h"

using namespace osgTerrain;


void xzPlaneLocator::setTransformAsExtents(double minX, double minZ, double maxX, double maxZ)
{
    _transform.set(maxX - minX, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, maxZ - minZ, 0.0,
        minX, 0.0, minZ, 1.0);

    _inverse.invert(_transform);
}
