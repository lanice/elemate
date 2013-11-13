#include <osg/ref_ptr>

namespace osg {
    class Vec3f;
    class Array;
}
namespace osgTerrain {
    class Terrain;
    class TerrainTile;
}

class TerrainGenerator {
public:
    TerrainGenerator(int numColumns = 1000, int numRows = 1000);
    osg::ref_ptr<osgTerrain::Terrain> getTerrain();
protected:
    osg::ref_ptr<osgTerrain::TerrainTile> createTile(double xyScale, float heightScale, float heightSigma);
    int m_numColumns;
    int m_numRows;
};
