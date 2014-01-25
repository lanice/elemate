#pragma once

#include <cassert>
#include <initializer_list>
#include <limits>
#include <functional>
#include <string>


enum class TerrainLevel {
    BaseLevel,
    WaterLevel
};
extern std::initializer_list<TerrainLevel> TerrainLevels;

void initMaterialTerrainLevels();

/** @return the terrain level that holds the material with name materialName */
TerrainLevel levelForMaterial(const std::string & materialName);

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
    /** Maximal height variance from terrain profil. This value is used to give the terrain slightly random structure. */
    float maxBasicHeightVariance;
    /** number of sample points along the x axis in one tile */
    unsigned rows;
    /** number of sample points along the z axis in one tile */
    unsigned columns;
    /** number of tiles along the x axis */
    unsigned tilesX;
    /** number of tiles along the z axis */
    unsigned tilesZ;
    /** size of one tile along the x axis */
    inline float tileSizeX() const { assert(tilesX >= 1); return sizeX / tilesX; };
    /** size of one tile along the z axis */
    inline float tileSizeZ() const { assert(tilesZ >= 1); return sizeZ / tilesZ; };
    /** number of sample points along the x axis in the hole terrain */
    unsigned samplesX() const {
        assert(((long long) rows*tilesX) < std::numeric_limits<unsigned>::max());
        return rows * tilesX;
    }
    /** number of sample points along the z axis in the hole terrain */
    unsigned samplesZ() const {
        assert(((long long) columns*tilesZ) < std::numeric_limits<unsigned>::max());
        return columns * tilesZ;
    }
    /** number of samples per world coordinate */
    inline float samplesPerWorldCoord() const {
        assert(sizeX > 0 && sizeZ > 0);
        assert(std::abs(rows / sizeX - columns / sizeZ) < std::numeric_limits<float>::epsilon());
        return rows / sizeX;
    }
    /** distance between two sample points along one axis */
    inline float sampleInterval() const {
        assert(rows >= 2 && columns >= 2);
        assert(std::abs(tileSizeX() / (rows - 1) - tileSizeZ() / (columns - 1)) < std::numeric_limits<float>::epsilon());
        return tileSizeX() / (rows - 1);
    }
};
