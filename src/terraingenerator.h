#pragma once

#include <cassert>
#include <map>
#include <osg/ref_ptr>
#include <foundation/PxSimpleTypes.h>

namespace osg {
    class Vec3f;
    class Array;
    class Geometry;
    class HeightField;
    class MatrixTransform;
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
    class PxMat44;
    class PxTransform;
}

struct TerrainSettings {
    TerrainSettings();
    float sizeX;
    float sizeY;
    unsigned columns;
    unsigned rows;
    unsigned tilesX;
    unsigned tilesY;
    inline float samplesPerXCoord() const { assert(sizeX > 0); return columns / sizeX; }
    inline float samplesPerYCoord() const { assert(sizeY > 0); return rows / sizeY; }
    inline float intervalX() const { assert(columns > 1); return sizeX / (columns - 1); }
    inline float intervalY() const { assert(rows > 1); return sizeY / (rows - 1); }
};


/** Terrain class holds osg and physx terrain objects and stores heighfield parameters.
**/
class ElemateHeightFieldTerrain {
public:
    explicit ElemateHeightFieldTerrain(const TerrainSettings & settings);

    /** osg transform node with terrain as child node
      * transforms the terrain to physx/opengl coordinates */
    osg::MatrixTransform * osgTransformedTerrain() const;
    /** osg terrain object containing terrain tiles with hight fields */
    osgTerrain::Terrain * osgTerrain() const;
    physx::PxShape const * pxShape(const osgTerrain::TileID & tileID) const;
    physx::PxRigidStatic * pxActor(const osgTerrain::TileID & tileID) const;

private:
    /** osg terrain object that can consist of multiple tiles */
    osg::ref_ptr<osgTerrain::Terrain> m_osgTerrain;

    /** osg transform to bring the terrain in physx world */
    osg::ref_ptr<osg::MatrixTransform> m_osgTerrainTransform;

    /** physx height field shape per terrain tile */
    std::map<osgTerrain::TileID, physx::PxShape*> m_pxShapes;
    /** physx static actor per terrain tile */
    std::map<osgTerrain::TileID, physx::PxRigidStatic*> m_pxActors;

    /** stores terrain configuration, set up by terrain generator */
    const TerrainSettings m_settings;

friend class TerrainGenerator;
};

class TerrainGenerator {
public:
    /** creates a generator with default settings */
    TerrainGenerator();
    /** terrain size in world coordinates */
    void setExtentsInWorld(float x, float y);
    float xExtens() const;
    float yExtens() const;
    /** height values per world coordinate 
      * The applied value may be a bit different as the number of total samples is an integral value. */
    void setSamplesPerWorldCoord(float xySamples);
    float samplesPerWorldCoord() const;
    void setTilesPerAxis(unsigned x, unsigned y);
    float tilesPerXAxis() const;
    float tilesPerYAxis() const;

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
    physx::PxShape * createPxShape(const osg::HeightField & osgHeightField, physx::PxRigidStatic & pxActor, const physx::PxTransform & scaling, const physx::PxMat44 & transform) const;

    //osg::ref_ptr<osgTerrain::Terrain> getTerrain();
    //ElemateHeightFieldTerrain * createHeightFieldTerrain();
    //osg::ref_ptr<osgTerrain::TerrainTile> createTile(double xyScale, float heightScale, float heightSigma);

    //int m_numColumns;
    //int m_numRows;
    //ElemateHeightFieldTerrain * m_elemateTerrain;
};
