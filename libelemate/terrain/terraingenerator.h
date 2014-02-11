#pragma once

#include <memory>

#include <glow/Array.h>

#include "terrainsettings.h"

namespace physx {
    class PxShape;
    class PxRigidStatic;
    struct PxHeightFieldSample;
}

class Terrain;
class TerrainTile;
class BaseTile;
class World;

/** Generator for height field terrains
  * Terrains are oriented in the xz-plane, using the default PhysX coordinate system for heightfields.
  * x is right, y is up, z is back
  * rows = x axis, height = y axis, columns = z axis */
class TerrainGenerator {
public:
    /** applies all settings and creates the height field landscape
      * all drawable have to know the world, give me the world, so that i can give it to my creations */
    std::shared_ptr<Terrain> generate(const World & world) const;

    /** terrain size in world coordinates */
    void setExtentsInWorld(float x, float z);
    float xExtens() const;
    float zExtens() const;
    /** Set number of rows and columns in height field depending on the sample rate.
      * The applied value may be a bit different as the number of total samples is an integral value. */
    void applySamplesPerWorldCoord(float samplesPerWorldCoord);
    float samplesPerWorldCoord() const;
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
    /** Maximal height variance from terrain profil. This value is used to give the terrain a slightly random structure.
      * Must be lower or equal than maxHeight, but should only be a fraction of it.*/
    void setMaxBasicHeightVariance(float variance);
    /** Maximal height variance from terrain profil. This value is used to give the terrain slightly random structure. */
    float maxBasicHeightVariance() const;
private:
    TerrainSettings m_settings;

    /** creates heightfield data with random height shift */
    glow::FloatArray * createBasicHeightField(float maxHeightVariance) const;
    /** adds a river bed to the heightField, getting the rock and sand indices from the initializer list
      * @return terrain type id storage in row major order */
    glow::UByteArray * gougeRiverBed(glow::FloatArray & heightField, const std::initializer_list<std::string> & baseElements) const;
    /** http://www.gameprogrammer.com/fractal.html#diamond algorithm for terrain creation */
    void diamondSquare(TerrainTile & tile) const;
    /** apply sand, grassland and bedrock terrain elements depending on the height values */
    void applyElementsByHeight(BaseTile & tile) const;
};
