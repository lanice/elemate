#include "terrainsettings.h"

#include <unordered_map>


std::initializer_list<TerrainLevel> TerrainLevels = {
    TerrainLevel::BaseLevel,
    TerrainLevel::WaterLevel
};

std::unordered_map<const std::string, TerrainLevel, std::hash<std::string>> s_elementToLevel;

void initElementTerrainLevels()
{
    s_elementToLevel.emplace("water", TerrainLevel::WaterLevel);
    s_elementToLevel.emplace("lava", TerrainLevel::WaterLevel);

    s_elementToLevel.emplace("bedrock", TerrainLevel::BaseLevel);
    s_elementToLevel.emplace("grassland", TerrainLevel::BaseLevel);
    s_elementToLevel.emplace("sand", TerrainLevel::BaseLevel);
}

TerrainLevel levelForElement(const std::string & elementName)
{
    assert(s_elementToLevel.find(elementName) != s_elementToLevel.end());
    return s_elementToLevel.at(elementName);
}

TerrainSettings::TerrainSettings()
: sizeX(400)
, sizeZ(400)
, maxHeight(40.f)
, tileSamplesPerAxis(1025)
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
