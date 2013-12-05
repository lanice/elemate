#pragma once

#include "elemateterrain.h"

namespace osgTerrain {
    class TileID;
    class TerrainTile;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
    struct PxHeightFieldSample;
}

class ElemateHeightFieldTerrain;
struct TerrainSettings;

/** Generator for height field terrains
  * Creates height field data and wraps it into PhysX and OSG objects. 
  * Terrains are oriented in the xz-plane, using the default PhysX coordinate system for heightfields.
  * x is right, y is up, z is back
  * Creates a transform node for the osg terrain object, to transform it in our cordinate system.
  * rows = x axis, height = y axis, columns = z axis
  * This is PhysX logic, OSG is kind of inverse, so don't get confused when reading the cpp =) */
class TerrainGenerator {
public:
    /** creates a generator with default settings */
    TerrainGenerator();
    /** applies all settings and creates the height field osg and physx objects */
    ElemateHeightFieldTerrain * generate() const;

    /** terrain size in world coordinates */
    void setExtentsInWorld(float x, float z);
    float xExtens() const;
    float zExtens() const;
    /** Set number of rows and columns in height field so that the terrain gets xzSamples of height values per world coordinate.
      * The applied value may be a bit different as the number of total samples is an integral value. */
    void applySamplesPerWorldCoord(float xzSamplesPerCoord);
    float samplesPerWorldXCoord() const;
    float samplesPerWorldZCoord() const;
    void setTilesPerAxis(unsigned x, unsigned z);
    int tilesPerXAxis() const;
    int tilesPerZAxis() const;

    /** generation time specific settings */

    /** maximum latitude (y value) in world coordinates
      *  -maxHeight < y <= maxHeight */
    void setMaxHeight(float height);
    /** maximum latitude (y value) in world coordinates
      * -maxHeight < y <= maxHeight */
    float maxHeight() const;
    /** set average size of one biome/terrain type */
    void setBiomeSize(float xzBiomeSize);
    /** average size of one biome/terrain type */
    float biomeSize() const;
private:
    TerrainSettings m_settings;

    /** creates a terrain tile, and sets its tileID */
    physx::PxHeightFieldSample * createPxHeightFieldData(unsigned numSamples) const;
    physx::PxShape * createPxShape(physx::PxRigidStatic & pxActor, const physx::PxHeightFieldSample * hfSamples) const;
    osgTerrain::TerrainTile * createTile(const osgTerrain::TileID & tileID, const physx::PxHeightFieldSample * pxHeightFieldSamples) const;
    /** creates biome data, adds it as vertex attrib array osg tile, sets pxSample textures accordingly */
    void createBiomes(osgTerrain::TerrainTile & tile, physx::PxHeightFieldSample * pxHeightFieldSamples) const;

    void operator=(TerrainGenerator& ) = delete;
};
