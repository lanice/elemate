#pragma once

#include <cassert>
#include <map>
#include <osg/ref_ptr>
#include <foundation/PxSimpleTypes.h>

namespace osg {
    class MatrixTransform;
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
    class PxMat44;
    class PxTransform;
}

struct TerrainSettings {
    TerrainSettings();
    float sizeX;
    float sizeZ;
    unsigned columns;
    unsigned rows;
    unsigned tilesX;
    unsigned tilesZ;
    inline float samplesPerXCoord() const { assert(sizeX > 0); return columns / sizeX; }
    inline float samplesPerZCoord() const { assert(sizeZ > 0); return rows / sizeZ; }
    inline float intervalX() const { assert(columns > 1); return sizeX / (columns - 1); }
    inline float intervalZ() const { assert(rows > 1); return sizeZ / (rows - 1); }
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
    /** PhysX shape containing height field geometry for one tile
      * terrain tile in origin is identified by TileId(0, 0, 0) */
    physx::PxShape const * pxShape(const osgTerrain::TileID & tileID) const;
    /** static PhysX actor for specified terrain tile 
      * terrain tile in origin is identified by TileId(0, 0, 0) */
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


/** Generator for height field terrains
  * Creates height field data and wraps it into osg and PhysX objects. 
  * Terrains are oriented in the xz-plane, using the default PhysX coordinate system for heightfields.
  * Creates a transform node for the osg terrain object, because osg terrains are aligned along the xy plane. */
class TerrainGenerator {
public:
    /** creates a generator with default settings */
    TerrainGenerator();
    /** terrain size in world coordinates */
    void setExtentsInWorld(float x, float z);
    float xExtens() const;
    float zExtens() const;
    /** Set number of columns and rows in height field so that the terrain gets xzSamples of height values per world coordinate.
      * The applied value may be a bit different as the number of total samples is an integral value. */
    void setSamplesPerWorldXCoord(float xSamples);
    void setSamplesPerWorldZCoord(float zSamples);
    float samplesPerWorldXCoord() const;
    float samplesPerWorldZCoord() const;
    void setTilesPerAxis(unsigned x, unsigned z);
    int tilesPerXAxis() const;
    int tilesPerZAxis() const;

    /** generation specific settings */

    /** sigma parameter for used normal distribution */
    void setHeightSigma(float sigma);
    /** sigma parameter for used normal distribution */
    float heightSigma() const;
    /** maximum latitude (y value) in world coordinates
      *  -maxHeight < y <= maxHeight */
    void setMaxHeight(float height);
    /** maximum latitude (y value) in world coordinates
      * -maxHeight < y <= maxHeight */
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
};
