#pragma once

#include <unordered_map>
#include <memory>

#include <glow/ref_ptr.h>
#include <glow/Array.h>

#include "terrainsettings.h"

namespace glow {
    class VertexArrayObject;
    class Buffer;
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

    friend class TerrainGenerator;

protected:
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

    friend class TerrainTile;

private:
    Terrain() = delete;
    void operator=(Terrain&) = delete;

};
