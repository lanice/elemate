#pragma once

#include <memory>

#include <glow/ref_ptr.h>
#include <glow/Array.h>

namespace glow {
    class VertexArrayObject;
    class Buffer;
    class Program;
    class Texture;
}
namespace glowutils {
    class Camera;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
}


enum class TerrainLevel {
    BaseLevel,
    WaterLevel
};
extern std::initializer_list<TerrainLevel> TerrainLevels;

struct TileID {
    TileID(TerrainLevel level = TerrainLevel::BaseLevel, int xID = 0, int zID = 0)
    : level(level), x(xID), z(zID)
    {}
    TerrainLevel level;
    unsigned int x;
    unsigned int z;
};

struct TerrainSettings {
    TerrainSettings();
    float sizeX;
    float sizeZ;
    /** maximal possible height value in terrain, as used in the generator */
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
    /** number of sample columns per x coordinate */
    inline float samplesPerXCoord() const { assert(sizeX > 0); return rows / sizeX; }
    /** number of sample rows per z coordinate */
    inline float samplesPerZCoord() const { assert(sizeZ > 0); return columns / sizeZ; }
    /** distance between two sample points along the x axis */
    inline float intervalX() const { assert(rows >= 2); return sizeX / (rows - 1); }
    /** distance between two sample points along the z axis */
    inline float intervalZ() const { assert(columns >= 2); return sizeZ / (columns - 1); }
};

class Terrain {
public:
    Terrain(const TileID & tileID, const TerrainSettings & settings);
    virtual ~Terrain();

    virtual void draw(const glowutils::Camera & camera);

protected:
    void initialize();

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_vbo;
    glow::ref_ptr<glow::Texture> m_heightTex;
    glow::ref_ptr<glow::Program> m_program;

    glow::FloatArray * m_heightField;
    glow::Vec2Array * m_vertices;

    physx::PxRigidStatic * m_pxActor;
    physx::PxShape * m_pxShape;

    const TerrainSettings m_settings;
    const TileID m_tileID;

    glm::mat4 m_transform;

    /** lowest tile id in x direction */
    unsigned minTileXID;
    /** lowest tile id in z direction */
    unsigned minTileZID;
    /** scaling factor to get physx integer height from world float height value */
    float heightScaleToPhysx;

    friend class TerrainGenerator;

private:
    Terrain() = delete;
    void operator=(Terrain&) = delete;
};
