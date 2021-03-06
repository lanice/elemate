#pragma once

#include "rendering/shadowingdrawable.h"

#include <map>
#include <set>
#include <memory>
#include <vector>

#include <glow/global.h>
#include <glow/ref_ptr.h>
#include <glowutils/AxisAlignedBoundingBox.h>
#include <glowutils/CachedValue.h>

#include "terrainsettings.h"

namespace physx {
    class PxRigidStatic;
}
class TerrainTile;
class PhysicalTile;

/** @brief base terrain object containing multiple tiles in different terrain levels.

    The TerrainGenerator can be used to create an instance of this class.
    Implements the scene and shadow rendering for the terrain tiles.*/
class Terrain : public ShadowingDrawable
{
public:
    Terrain(const TerrainSettings & settings);

    /** set a list of elements that will be used for the draw call */
    virtual void draw(const CameraEx & camera, const std::initializer_list<std::string> & elements);
    virtual void drawDepthMap(const CameraEx & camera, const std::initializer_list<std::string> & elements);
    virtual void drawShadowMapping(const CameraEx & camera, const CameraEx & lightSource, const std::initializer_list<std::string> & elements);

    /** @return interpolated maximal height of all terrain layers at specific world position */
    float heightTotalAt(float x, float z) const;
    /** @return interpolated height at specific world position in a specific terrain level */
    float heightAt(float x, float z, TerrainLevel level) const;
    /** @return highest terrain level at position */
    TerrainLevel heighestLevelAt(float x, float z) const;
    void heighestLevelHeightAt(float x, float z, TerrainLevel & maxLevel, float & maxHeight) const;
    /** @return the bounding box reduced by the border width */
    const glowutils::AxisAlignedBoundingBox & validBoundingBox() const;
    /** Access settings object. This only stores values from creation time and cannot be changed. */
    const TerrainSettings settings;

    void updatePhysics(double delta);

    void setDrawHeatMap(bool drawHeatMap);

    void setDrawGridOffsetUniform(glow::Program & program, const glm::vec3 & cameraposition) const;

    friend class TerrainGenerator;
    friend class TerrainTile;
    friend class TerrainInteraction;

protected:
    /** Distance from camera to farthest visible point. The rendered terrain size depends on this parameter.
      * The terrain will adjust this parameter internally on each draw call, depending on the camera's zfar. */
    void setViewRange(float zfar);

    void setDrawElements(const std::initializer_list<std::string> & elements);
    std::set<TerrainLevel> m_drawLevels;

    virtual void drawImplementation(const CameraEx & camera) override;
    virtual void drawDepthMapImpl(const CameraEx & camera) override;
    virtual void drawShadowMappingImpl(const CameraEx & camera, const CameraEx & lightSource) override;

    /** register terrain tile to be part of this terrain with unique tileID */
    void registerTile(const TileID & tileID, TerrainTile & tile);

    std::map<TileID, std::shared_ptr<TerrainTile>> m_physicalTiles;
    std::map<TileID, std::shared_ptr<TerrainTile>> m_attributeTiles;
    std::shared_ptr<TerrainTile> getTile(TileID tileID) const;

    /** holds one physx actor per tile x/z-ID. TileId.level is always BaseLevel */
    std::map<TileID, physx::PxRigidStatic*> m_pxActors;

    /** lowest tile id in x direction */
    unsigned minTileXID;
    /** lowest tile id in z direction */
    unsigned minTileZID;

    /** Distance from camera to farthest visible point. The rendered terrain size depends on this parameter. */
    float m_viewRange;
    glowutils::CachedValue<glowutils::AxisAlignedBoundingBox> m_validBoudingBox;

    virtual void initialize() override;
    void generateDrawGrid();

    glowutils::CachedValue<unsigned int> m_renderGridRadius;
    std::vector<glm::vec2> m_vertices;
    std::vector<uint32_t> m_indices;

    /** light map and shadow mapping */
    virtual void initDepthMapProgram() override;
    virtual void initShadowMappingProgram() override;

    static const GLuint s_restartIndex;

protected:
    /** Fetch the tile corresponding to the xz world coordinates and the terrain level and sets the row/column position in this tile.
    * @param terrainTile if world x/z position are in range, this pointer will be set to a valid terrain tile.
    * @return true, if the position is in terrain extent's range. */
    bool worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column) const;
    bool worldToTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<TerrainTile> & terrainTile, unsigned int & row, unsigned int & column, float & row_fract, float & column_fract) const;
    bool worldToPhysicalTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<PhysicalTile> & physicalTile, unsigned int & row, unsigned int & column) const;
    bool worldToPhysicalTileRowColumn(float x, float z, TerrainLevel level, std::shared_ptr<PhysicalTile> & physicalTile, unsigned int & row, unsigned int & column, float & row_fract, float & column_fract) const;
    /** transform world position into tileID and normalized coordinates in this tile.
    * @param tileID this will set the x, y values of the id, but will not change the level
    * @param normX normZ these parameter will be set the normalized position in the tile, referenced with tileID
    * @return whether the world position is in range of the terrain. The tileID does only reference a valid tile if the function returns true. */
    bool normalizePosition(float x, float z, TileID & tileID, float & normX, float & normZ) const;

public:
    void operator=(Terrain&) = delete;

};
