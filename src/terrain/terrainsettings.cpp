#include "terrainsettings.h"


std::initializer_list<TerrainLevel> TerrainLevels = {
    TerrainLevel::BaseLevel,
    TerrainLevel::WaterLevel
};

TerrainSettings::TerrainSettings()
: sizeX(200)
, sizeZ(200)
, maxHeight(20.f)
, maxBasicHeightVariance(0.05f)
, rows(20)
, columns(20)
, tilesX(1)
, tilesZ(1)
{
}

TileID::TileID(TerrainLevel level /*= TerrainLevel::BaseLevel*/, int xID /*= 0*/, int zID /*= 0*/)
: level(level), x(xID), z(zID)
{}

bool std::less<TileID>::operator()(const TileID& lhs, const TileID& rhs) const
{
    return
    (lhs.level != rhs.level)
     ? static_cast<unsigned int>(lhs.level)
        < static_cast<unsigned int>(rhs.level)
     : (lhs.x != rhs.x)
     	? (lhs.x < rhs.x)
     	: (lhs.z < rhs.z);
}
