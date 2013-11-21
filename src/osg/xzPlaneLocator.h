
#include <osgTerrain/Locator>

namespace osgTerrain {

    /** Overrides some functions of osgTerrain::Locator to work with the OpenGL/PhysX coordinate system
      *     x is right
      *     y is up
      *    -z is front
      */
    class xzPlaneLocator : public Locator {
    public:

        META_Object(osgTerrain, Locator);

        // non virtual function!
        void setTransformAsExtents(double minX, double minZ, double maxX, double maxZ);
    };

}
