#pragma once

#include "rendering/drawable.h"

#include <map>
#include <memory>

#include <glow/ref_ptr.h>
#include <glow/Array.h>

#include "terrainsettings.h"

namespace physx {
    class PxShape;
    class PxRigidStatic;
}
class TerrainTile;

class Terrain : public Drawable
{
public:
    Terrain(const World & world, const TerrainSettings & settings);
    virtual ~Terrain() override;

    /** PhysX shape containing height field geometry for one tile.
    * Terrain tile in origin is identified by TileId(0, 0, 0) */
    physx::PxShape * pxShape(const TileID & tileID) const;
    /** static PhysX actor for terrain tiles at specified x/z-ID.
    * Terrain tile in origin is identified by TileId(0, 0, 0) */
    physx::PxRigidStatic * pxActor(const TileID & tileID) const;
    /** Map of static PhysX actors. TileID's level is always BaseLevel.
      * One actor owns the shapes for all terain levels at its x/z-ID. */
    const std::map<TileID, physx::PxRigidStatic*> pxActorMap() const;
    /** @return interpolated maximal height of all terrain layers at specific world position */
    float heightTotalAt(float x, float z) const;
    /** @return interpolated height at specific world position in a specific terrain level */
    float heightAt(float x, float z, TerrainLevel level) const;
    /** Access settings object. This only stores values from creation time and cannot be changed. */
    const TerrainSettings settings;

    friend class TerrainGenerator;
    friend class TerrainTile;
    friend class TerrainInteractor;

protected:
    virtual void drawImplementation(const glowutils::Camera & camera) override;
    virtual void drawLightMapImpl(const CameraEx & lightSource) override;
    virtual void drawShadowMappingImpl(const glowutils::Camera & camera, const CameraEx & lightSource) override;

    /** register terrain tile to be part of this terrain with unique tileID */
    void registerTile(const TileID & tileID, TerrainTile & tile);

    std::map<TileID, std::shared_ptr<TerrainTile>> m_tiles;
    /** holds one physx actor per tile x/z-ID. TileId.level is always BaseLevel */
    std::map<TileID, physx::PxRigidStatic*> m_pxActors;

    /** lowest tile id in x direction */
    unsigned minTileXID;
    /** lowest tile id in z direction */
    unsigned minTileZID;

    virtual void initialize() override;
    void generateVertices();
    void generateIndices();

    glow::Vec2Array * m_vertices;
    glow::UIntArray * m_indices;

    /** light map and shadow mapping */
    virtual void initLightMappingProgram() override;
    virtual void initShadowMappingProgram() override;

    static const GLuint s_restartIndex;

protected:

    /** Fetch the tile corresponding to the xz world coordinates and the terrain level and sets the row/column position in this tile.
    * @param terrainTile if world x/z position are in range, this pointer will be set to a valid terrain tile.
    * @return true, if the position is in terrain extent's range. */
    bool worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column) const;
    bool worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column, float & row_fract, float & column_fract) const;
    /** transform world position into tileID and normalized coordinates in this tile.
    * @param tileID this will set the x, y values of the id, but will not change the level
    * @param normX normZ these parameter will be set the normalized position in the tile, referenced with tileID
    * @return whether the world position is in range of the terrain. The tileID does only reference a valid tile if the function returns true. */
    bool normalizePosition(float x, float z, TileID & tileID, float & normX, float & normZ) const;

public:
    void operator=(Terrain&) = delete;

};
