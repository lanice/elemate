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

bool TileID::operator==(const TileID & other) const {
    return (level == other.level)
        && (x == other.x)
        && (z == other.z);
}

size_t std::hash<TileID>::operator()(const TileID& id) const
{
    return
        std::hash<int>()(static_cast<int>(id.level))
        ^ std::hash<unsigned int>()(static_cast<unsigned int>(id.x))
        ^ std::hash<unsigned int>()(static_cast<unsigned int>(id.z));
}
