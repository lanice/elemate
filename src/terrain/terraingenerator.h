#pragma once

#include <memory>

#include "terrainsettings.h"

namespace physx {
    class PxShape;
    class PxRigidStatic;
    struct PxHeightFieldSample;
}

class Terrain;
class TerrainTile;

/** Generator for height field terrains
  * Creates height field data and wraps it into PhysX and OSG objects. 
  * Terrains are oriented in the xz-plane, using the default PhysX coordinate system for heightfields.
  * x is right, y is up, z is back
  * Creates a transform node for the osg terrain object, to transform it in our cordinate system.
  * rows = x axis, height = y axis, columns = z axis
  * This is PhysX logic, OSG is kind of inverse, so don't get confused when reading the cpp =) */
class TerrainGenerator {
public:
    /** applies all settings and creates the height field osg and physx objects */
    std::shared_ptr<Terrain> generate() const;

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
      *  -maxHeight <= y <= maxHeight */
    void setMaxHeight(float height);
    /** maximum latitude (y value) in world coordinates
      * -maxHeight <= y <= maxHeight */
    float maxHeight() const;
    /** Maximal height variance from terrain profil. This value is used to give the terrain slightly random structure.
      * Must be lower or equal than maxHeight, but should only be a fraction of it.*/
    void setMaxBasicHeightVariance(float variance);
    /** Maximal height variance from terrain profil. This value is used to give the terrain slightly random structure. */
    float maxBasicHeightVariance() const;
private:
    TerrainSettings m_settings;

    /** creates physx and osg heightfield data with random height shift */
    physx::PxHeightFieldSample * createBasicPxHeightField(unsigned char defaultTerrainTypeId, float maxHeightVariance) const;
    /** adds a river bed to the shape */
    void gougeRiverBed(physx::PxHeightFieldSample * pxHfSamples) const;
    /** copies the height values to an osg heightfield layer and create a tile with it */
    void copyPxHeightFieldToTile(TerrainTile & tile, const physx::PxHeightFieldSample * pxHeightFieldSamples) const;
    physx::PxShape * createPxShape(physx::PxRigidStatic & pxActor, const physx::PxHeightFieldSample * hfSamples) const;

    void operator=(TerrainGenerator& ) = delete;
};
