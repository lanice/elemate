#include "terrainsettings.h"

#include <algorithm>

namespace {
    std::unordered_map<const std::string, TerrainLevel, std::hash<std::string>> initElementTerrainLevels()
    {
        std::unordered_map<const std::string, TerrainLevel, std::hash<std::string>> l_elementToLevel;

        l_elementToLevel.emplace("water", TerrainLevel::WaterLevel);
        l_elementToLevel.emplace("lava", TerrainLevel::WaterLevel);

        l_elementToLevel.emplace("bedrock", TerrainLevel::BaseLevel);
        l_elementToLevel.emplace("grassland", TerrainLevel::BaseLevel);
        l_elementToLevel.emplace("sand", TerrainLevel::BaseLevel);

        l_elementToLevel.emplace("temperature", TerrainLevel::TemperatureLevel);

        return l_elementToLevel;
    }

}

const std::unordered_map<const std::string, TerrainLevel, std::hash<std::string>> levelForElement = initElementTerrainLevels();

const std::initializer_list<TerrainLevel> PhysicalLevels = {
    TerrainLevel::BaseLevel,
    TerrainLevel::WaterLevel,
};

const std::initializer_list<TerrainLevel> AttributeLevels = {
    TerrainLevel::TemperatureLevel
};

bool levelIsPhysical(TerrainLevel level)
{
    return std::find(PhysicalLevels.begin(), PhysicalLevels.end(), level) != PhysicalLevels.end();
}

bool levelIsAttribute(TerrainLevel level)
{
    return std::find(AttributeLevels.begin(), AttributeLevels.end(), level) != AttributeLevels.end();
}

TerrainSettings::TerrainSettings()
: sizeX(400)
, sizeZ(400)
, maxHeight(40.f)
, maxTileSamplesPerAxis(1025)
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
