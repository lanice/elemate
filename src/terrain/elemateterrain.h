#pragma once

#include <cassert>
#include <limits>
#include <map>

#include <osg/ref_ptr>
#include <osg/Shape>

#include <geometry/PxHeightFieldSample.h>

namespace osg {
    class MatrixTransform;
}
namespace osgTerrain {
    class Terrain;
    class TileID;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
}

struct TerrainSettings {
    TerrainSettings();
    float sizeX;
    float sizeZ;
    /** maximal possible height value in terrain, as used in the generator */
    float maxHeight;
    /** Maximal height variance from terrain profil. This value is used to give the terrain slightly random structure. */
    float maxBasicHeightVariance;
    /** number of sample points along the x axis in one tile */
    unsigned rows;
    /** number of sample points along the z axis in one tile */
    unsigned columns;
    /** number of tiles along the x axis */
    unsigned tilesX;
    /** number of tiles along the z axis */
    unsigned tilesZ;
    /** size of one tile along the x axis */
    inline float tileSizeX() const { assert(tilesX >= 1); return sizeX / tilesX; };
    /** size of one tile along the z axis */
    inline float tileSizeZ() const { assert(tilesZ >= 1); return sizeZ / tilesZ; };
    /** number of sample points along the x axis in the hole terrain */
    unsigned samplesX() const {
        assert(((long long) rows*tilesX) < std::numeric_limits<unsigned>::max());
        return rows * tilesX;
    }
    /** number of sample points along the z axis in the hole terrain */
    unsigned samplesZ() const {
        assert(((long long) columns*tilesZ) < std::numeric_limits<unsigned>::max());
        return columns * tilesZ;
    }
    /** number of sample columns per x coordinate */
    inline float samplesPerXCoord() const { assert(sizeX > 0); return rows / sizeX; }
    /** number of sample rows per z coordinate */
    inline float samplesPerZCoord() const { assert(sizeZ > 0); return columns / sizeZ; }
    /** distance between two sample points along the x axis */
    inline float intervalX() const { assert(rows >= 2); return sizeX / (rows - 1); }
    /** distance between two sample points along the z axis */
    inline float intervalZ() const { assert(columns >= 2); return sizeZ / (columns - 1); }
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
    /** map of static PhysX actors */
    const std::map<osgTerrain::TileID, physx::PxRigidStatic*> pxActorMap() const;
    /** @return height at specific world position */
    float heightAt(float x, float z) const;
    /** sets the physx and osg heightfield value in a specific tile at (physx) row/column position (means in x/z direction)
        @return false, if row/column out of range */
    bool setHeightInTile(const osgTerrain::TileID & tileID, unsigned int physxRow, unsigned int physxColumn, float height);
    /** sets the physx and osg heightfield value in world coordinates x/z 
        @return false, if x or z out of range */
    bool setHeight(unsigned int x, unsigned int z, float height);
    /** Access settings object. This only stores values from creation time and cannot be changed. */
    const TerrainSettings & settings() const;

private:
    /** osg terrain object that can consist of multiple tiles */
    osg::ref_ptr<osgTerrain::Terrain> m_osgTerrain;

    /** osg transform to bring the terrain in physx world */
    osg::ref_ptr<osg::MatrixTransform> m_osgTerrainTransform;

    /** physx height field shape per terrain tile */
    std::map<osgTerrain::TileID, physx::PxShape*> m_pxShapes;
    /** physx static actor per terrain tile */
    std::map<osgTerrain::TileID, physx::PxRigidStatic*> m_pxActors;

    /** Sets the physx and osg heightfield value at equivalent positions to the data arrays.
        Doesn't perfome any range checks, so make sure to pass valid row/column and height value!
        Height scaling depends on heightScaleToPhysx, row/column conversion on current terrain settings. */
    inline void setHeight(physx::PxHeightFieldSample * pxHeightField, osg::HeightField * osgHeightField, unsigned int physxRow, unsigned int physxColumn, float worldHeight)
    {
        assert(physxRow >= 0 && physxRow < m_settings.rows); assert(physxColumn >= 0 && physxColumn < m_settings.columns);
        osgHeightField->setHeight(physxRow, m_settings.columns - physxColumn - 1, worldHeight);
        pxHeightField[physxColumn + m_settings.columns*physxRow].height = static_cast<physx::PxI16>(worldHeight * heightScaleToPhysx);
    }

    /** stores terrain configuration, set up by terrain generator */
    const TerrainSettings m_settings;

    /** lowest tile id in x direction */
    unsigned minTileXID;
    /** lowest tile id in z direction */
    unsigned minTileZID;
    /** scaling factor to get physx integer height from world float height value */
    float heightScaleToPhysx;

    ElemateHeightFieldTerrain & operator=(const ElemateHeightFieldTerrain &) = delete;

    friend class TerrainGenerator;
};
