#include <osg/ref_ptr>

namespace osg {
    class Vec3f;
    class Array;
    class Geometry;
}
namespace osgTerrain {
    class Terrain;
    class TerrainTile;
}
namespace physx {
    class PxTriangleMeshGeometry;
}

class TerrainGenerator {
public:
    TerrainGenerator(int numColumns = 1000, int numRows = 1000);
    osg::ref_ptr<osgTerrain::Terrain> getTerrain();
    static physx::PxTriangleMeshGeometry * pxTerrainGeometry(const osgTerrain::Terrain * terrain);
protected:
    osg::ref_ptr<osgTerrain::TerrainTile> createTile(double xyScale, float heightScale, float heightSigma);
    int m_numColumns;
    int m_numRows;
};
