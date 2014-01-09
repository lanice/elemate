#pragma once

#include <unordered_map>
#include <memory>

#include <glow/ref_ptr.h>
#include <glow/Array.h>

#include "terrainsettings.h"

namespace glow {
    class VertexArrayObject;
    class Buffer;
    class Program;
}
namespace glowutils {
    class Camera;
}
namespace physx {
    class PxShape;
    class PxRigidStatic;
}

class TerrainTile;

class Terrain {
public:
    Terrain(const TerrainSettings & settings);
    virtual ~Terrain();

    virtual void draw(const glowutils::Camera & camera);
    /** writes the linearized depth into the current depth attachment */
    virtual void drawShadow(const glowutils::Camera & lightSource);

    /** PhysX shape containing height field geometry for one tile.
    * Terrain tile in origin is identified by TileId(0, 0, 0) */
    physx::PxShape * pxShape(const TileID & tileID) const;
    /** static PhysX actor for terrain tiles at specified x/z-ID.
    * Terrain tile in origin is identified by TileId(0, 0, 0) */
    physx::PxRigidStatic * pxActor(const TileID & tileID) const;
    /** Map of static PhysX actors. TileID's level is always BaseLevel.
      * One actor owns the shapes for all terain levels at its x/z-ID. */
    const std::unordered_map<TileID, physx::PxRigidStatic*> pxActorMap() const;
    /** @return interpolated height at specific world position */
    float heightAt(float x, float z) const;
    /** @return interpolated height at specific world position in a specific terrain level */
    float heightAt(float x, float z, TerrainLevel level) const;
    /** Access settings object. This only stores values from creation time and cannot be changed. */
    const TerrainSettings settings;

    /** sets the lighting uniforms for terrain tile shaders - copy paste from World */
    void setUpLighting(glow::Program & program) const;

    friend class TerrainGenerator;
    friend class TerrainTile;
    friend class TerrainInteractor;

protected:
    /** register terrain tile to be part of this terrain with unique tileID */
    void registerTile(const TileID & tileID, TerrainTile & tile);

    std::unordered_map<TileID, std::shared_ptr<TerrainTile>> m_tiles;
    /** holds one physx actor per tile x/z-ID. TileId.level is always BaseLevel */
    std::unordered_map<TileID, physx::PxRigidStatic*> m_pxActors;

    /** lowest tile id in x direction */
    unsigned minTileXID;
    /** lowest tile id in z direction */
    unsigned minTileZID;

    void initialize();
    void generateVertices();
    void generateIndices();

    glow::ref_ptr<glow::VertexArrayObject> m_vao;
    glow::ref_ptr<glow::Buffer> m_indexBuffer;
    glow::ref_ptr<glow::Buffer> m_vbo;

    glow::Vec2Array * m_vertices;
    glow::UIntArray * m_indices;

    /** light map and shadow mapping */
    void initShadowProgram();
    glow::ref_ptr<glow::Program> m_shadowProgram;

protected:

    /** Fetch the tile corresponding to the xz world coordinates and the terrain level and sets the row/column position in this tile.
    * @param terrainTile if world x/z position are in range, this pointer will be set to a valid terrain tile.
    * @return true, if the position is in terrain extent's range. */
    bool worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column) const;
    /** transform world position into tileID and normalized coordinates in this tile.
    * @param tileID this will set the x, y values of the id, but will not change the level
    * @param normX normZ these parameter will be set the normalized position in the tile, referenced with tileID
    * @return whether the world position is in range of the terrain. The tileID does only reference a valid tile if the function returns true. */
    bool normalizePosition(float x, float z, TileID & tileID, float & normX, float & normZ) const;

public:
    Terrain() = delete;
    void operator=(Terrain&) = delete;

};
