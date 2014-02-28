#pragma once

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <functional>
#include <string>
#include <cmath>


enum class TerrainLevel {
    BaseLevel,
    WaterLevel,
    TemperatureLevel
};
extern std::initializer_list<TerrainLevel> TerrainLevels;

/** initialize the list that defines which terrain level should hold which element */
void initElementTerrainLevels();

/** @return the terrain level that holds the element with name elementName */
TerrainLevel levelForElement(const std::string & elementName);

/** @return if this tile type has a physical/rendered representation */
bool levelIsPhysical(TerrainLevel level);

struct TileID {
    TileID(TerrainLevel level = TerrainLevel::BaseLevel, int xID = 0, int zID = 0);
    TerrainLevel level;
    unsigned int x;
    unsigned int z;
};

namespace std {
    /** level precedes x, precedes z */
    template<> struct less<TileID> {
        bool operator()(const TileID& lhs, const TileID& rhs) const;
    };
}

struct TerrainSettings {
    TerrainSettings();
    float sizeX;
    float sizeZ;
    /** Maximal absolute height value in terrain. */
    float maxHeight;
    /** maximal number of sample points along the x/y axes in one tile. TerrainTiles may use lower sample rate. */
    uint32_t maxTileSamplesPerAxis;
    /** number of tiles along the x axis */
    unsigned tilesX;
    /** number of tiles along the z axis */
    unsigned tilesZ;
    /** size of one tile along the x/z axes */
    inline float tileBorderLength() const {
        assert(tilesX >= 1 && tilesZ >= 1);
        assert(std::abs(sizeX / tilesX - sizeZ / tilesZ) < std::numeric_limits<float>::epsilon());
        return sizeX / tilesX;
    };
    /** number of samples per world coordinate for tiles that use the maximum sample rate */
    inline float maxSamplesPerWorldCoord() const {
        assert(sizeX > 0 && sizeZ > 0);
        assert(std::abs(maxTileSamplesPerAxis * tilesX / sizeX - maxTileSamplesPerAxis * tilesZ / sizeZ) < std::numeric_limits<float>::epsilon());
        return maxTileSamplesPerAxis * tilesX / sizeX;
    }
    /** distance between two sample points along one axis for tiles that use the maximum sample rate */
    inline float minSampleInterval() const {
        assert(maxTileSamplesPerAxis >= 2);
        return tileBorderLength() / (maxTileSamplesPerAxis - 1);
    }
};
