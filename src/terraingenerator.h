#include <osg/ref_ptr>
#include <foundation/PxSimpleTypes.h>

namespace osg {
    class Vec3f;
    class Array;
    class Geometry;
    class HeightField;
}
namespace osgTerrain {
    class Terrain;
    class TerrainTile;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
    class PxHeightField;
    class PxHeightFieldGeometry;
}

/** Terrain class holds osg and physx terrain objects and stores heighfield parameters.
**/
class ElemateHeightFieldTerrain {
public:
    ElemateHeightFieldTerrain();
    ~ElemateHeightFieldTerrain() {}
    // returns physx shape for heightField, creates it if necessary
    physx::PxShape * heightFieldShape();
    physx::PxRigidStatic * actor() const;
//protected:
    osg::ref_ptr<osgTerrain::Terrain> m_osgTerrain;
    osg::ref_ptr<osg::HeightField> m_osgHeightField;
    physx::PxRigidStatic * m_actor;
    physx::PxShape * m_shape;
    physx::PxHeightField * m_pxHeightField;
    physx::PxHeightFieldGeometry * m_pxHfGeometry;
    physx::PxU32 m_numRows;      // PhysX uses row-major order for heightfield
    physx::PxU32 m_numColumns;
    physx::PxReal m_heightScale;
    physx::PxReal m_rowScale;
    physx::PxReal m_colScale;
};

class TerrainGenerator {
public:
    TerrainGenerator(int numColumns = 1000, int numRows = 1000);
    osg::ref_ptr<osgTerrain::Terrain> getTerrain();
    ElemateHeightFieldTerrain * createHeightFieldTerrain();
    //static physx::PxTriangleMeshGeometry * pxTerrainGeometry(const osgTerrain::Terrain * terrain);
protected:
    osg::ref_ptr<osgTerrain::TerrainTile> createTile(double xyScale, float heightScale, float heightSigma);
    int m_numColumns;
    int m_numRows;
    ElemateHeightFieldTerrain * m_elemateTerrain;
};
