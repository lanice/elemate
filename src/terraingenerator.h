
#include <map>
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
    class TileID;
    class TerrainTile;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
    class PxHeightField;
    class PxHeightFieldGeometry;
}

struct TerrainSettings {
    TerrainSettings();
    float sizeX;
    float sizeZ;
    unsigned columns;
    unsigned rows;
    unsigned tilesX;
    unsigned tilesZ;
    float xSamplesPerCoord() const { return columns / sizeX; }
    float zSamplesPerCoord() const { return rows / sizeZ; }
};


/** Terrain class holds osg and physx terrain objects and stores heighfield parameters.
**/
class ElemateHeightFieldTerrain {
public:
    explicit ElemateHeightFieldTerrain(const TerrainSettings & settings);

    osgTerrain::Terrain * osgTerrain() const;
    physx::PxShape const * pxShape(const osgTerrain::TileID & tileID) const;
    physx::PxRigidStatic * pxActor(const osgTerrain::TileID & tileID) const;

private:
    /** osg terrain object that can consist of multiple tiles */
    osg::ref_ptr<osgTerrain::Terrain> m_osgTerrain;

    /** physx height field shape per terrain tile */
    std::map<osgTerrain::TileID, physx::PxShape*> m_pxShapes;
    /** physx static actor per terrain tile */
    std::map<osgTerrain::TileID, physx::PxRigidStatic*> m_pxActors;

    const TerrainSettings m_settings;

friend class TerrainGenerator;
};

class TerrainGenerator {
public:
    /** creates a generator with default settings */
    TerrainGenerator();
    /** terrain size in world coordinates */
    void setExtentsInWorld(float x, float z);
    float xExtens() const;
    float zExtens() const;
    /** height values per world coordinate 
      * The applied value may be a bit different as the number of total samples is an integral value. */
    void setSamplesPerWorldCoord(float xzSamples);
    float samplesPerWorldCoord() const;
    void setTilesPerAxis(unsigned x, unsigned z);
    float tilesPerXAxis() const;
    float tilesPerZAxis() const;

    /** generation specific settings */
    /** sigma parameter for used normal distribution */
    void setHeightSigma(float sigma);
    float heightSigma() const;
    void setMaxHeight(float height);
    float maxHeight() const;

    /** applies all settings and creates the height field osg and physx objects */
    ElemateHeightFieldTerrain * generate() const;

private:
    TerrainSettings m_settings;

    mutable ElemateHeightFieldTerrain * artifact;

    float m_heightSigma;
    float m_maxHeight;

    /** creates a terrain tile, and sets its tileID */
    osgTerrain::TerrainTile * createTile(const osgTerrain::TileID & tileID) const;
    physx::PxShape * createPxShape(const osg::HeightField & osgHeightField, physx::PxRigidStatic & pxActor) const;

    //osg::ref_ptr<osgTerrain::Terrain> getTerrain();
    //ElemateHeightFieldTerrain * createHeightFieldTerrain();
    //osg::ref_ptr<osgTerrain::TerrainTile> createTile(double xyScale, float heightScale, float heightSigma);

    //int m_numColumns;
    //int m_numRows;
    //ElemateHeightFieldTerrain * m_elemateTerrain;
};
