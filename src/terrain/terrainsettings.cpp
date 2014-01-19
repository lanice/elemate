#include "terrainsettings.h"

#include <unordered_map>


std::initializer_list<TerrainLevel> TerrainLevels = {
    TerrainLevel::BaseLevel,
    TerrainLevel::WaterLevel
};

std::unordered_map<const std::string, TerrainLevel, std::hash<std::string>> s_levelsForMaterials;

void initMaterialTerrainLevels()
{
    s_levelsForMaterials.emplace("water", TerrainLevel::WaterLevel);
    s_levelsForMaterials.emplace("lava", TerrainLevel::WaterLevel);

    s_levelsForMaterials.emplace("bedrock", TerrainLevel::BaseLevel);
    s_levelsForMaterials.emplace("grassland", TerrainLevel::BaseLevel);
    s_levelsForMaterials.emplace("dirt", TerrainLevel::BaseLevel);
}

TerrainLevel levelForMaterial(const std::string & materialName)
{
    assert(s_levelsForMaterials.find(materialName) != s_levelsForMaterials.end());
    return s_levelsForMaterials.at(materialName);
}

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
