#pragma once

#include <memory>

#include "terrainsettings.h"

class Terrain;
class TerrainTile;
class BaseTile;

/** Generator for height field terrains
  * Terrains are oriented in the xz-plane, using the default PhysX coordinate system for heightfields.
  * x is right, y is up, z is back
  * rows = x axis, height = y axis, columns = z axis */
class TerrainGenerator {
public:
    /** applies all settings and creates the height field landscape */
    std::shared_ptr<Terrain> generate() const;

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
private:
    TerrainSettings m_settings;

    /** http://www.gameprogrammer.com/fractal.html#diamond algorithm for terrain creation */
    void diamondSquare(TerrainTile & tile) const;
    /** apply sand, grassland and bedrock terrain elements depending on the height values */
    void applyElementsByHeight(BaseTile & tile) const;
};
