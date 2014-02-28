#pragma once

#include "terraintile.h"

class TemperatureTile : public TerrainTile
{
public:
    TemperatureTile(Terrain & terrain, const TileID & tileId);

    const static float minTemperature;
    const static float maxTemperature;
};
