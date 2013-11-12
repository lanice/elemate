#include <vector>
#include <osg/ref_ptr>

namespace osg {
    class Vec3f;
    class Array;
}
namespace osgTerrain {
    class Terrain;
}

class TerrainGenerator {
public:
    TerrainGenerator(int numColumns = 100, int numRows = 100);
    osg::ref_ptr<osgTerrain::Terrain> getTerrain();
protected:

    int m_numColumns;
    int m_numRows;
};
