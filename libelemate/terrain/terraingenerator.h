#pragma once

#include <memory>

#include "terrainsettings.h"

class Terrain;
class TerrainTile;
class BaseTile;

/** Generator for height field terrains
  * Terrains are oriented in the xz-plane, using the default PhysX coordinate system for height fields.
  * x is right, y is up, z is back
  * rows = x axis, height = y axis, columns = z axis */
class TerrainGenerator {
public:
    /** applies all settings and creates the height field landscape */
    std::shared_ptr<Terrain> generate() const;

private:
    TerrainSettings m_settings;

    /** http://www.gameprogrammer.com/fractal.html#diamond algorithm for terrain creation */
    void diamondSquare(TerrainTile & tile) const;
    /** apply sand, grassland and bedrock terrain elements depending on the height values */
    void applyElementsByHeight(BaseTile & tile) const;
};
