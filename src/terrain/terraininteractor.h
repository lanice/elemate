#pragma once

#include <memory>

#include "terrainsettings.h"

class Terrain;
class TerrainTile;

class TerrainInteractor {
public:
    TerrainInteractor(std::shared_ptr<Terrain>& terrain);

    float heightAt(float worldX, float worldZ) const;
    float heightAt(float worldX, float worldZ, TerrainLevel level) const;

    /** Set the terrain height at a specified world position to value.
      * @param value the new height value. Will be clamped to terrain's [-maxHeight, maxHeight] if necessary.
      * @return the new applied height value or zero if the position is out of range. */
    float setHeight(float worldX, float worldZ, TerrainLevel level, float value);
    /** Add delta to the terrain height at a specified world position.
      * The actual height value will be clampted to terrain's [-maxHeight, maxHeight] if necessary.
      * @return the new applied height value or zero if the position is out of range. */
    float changeHeight(float worldX, float worldZ, TerrainLevel level, float delta);

    /** grabs the terrain at worldXZ, storing the current height value */
    float heightGrab(float worldX, float worldZ, TerrainLevel level);
    /** pulls the terrain at worldXZ, setting the height to the grabbed value */
    void heightPull(float worldX, float worldZ);

    std::shared_ptr<const Terrain> terrain() const;
    void setTerrain(std::shared_ptr<Terrain>& terrain);

private:
    std::shared_ptr<Terrain> m_terrain;

    float setHeight(TerrainTile & tile, unsigned row, unsigned column, float value);
    void setPxHeight(TerrainTile & tile, unsigned row, unsigned column, float value);

    TerrainLevel m_grabbedLevel;
    float m_grabbedHeight;

public:
    TerrainInteractor(TerrainInteractor&) = delete;
    void operator=(TerrainInteractor&) = delete;
    TerrainInteractor() = delete;
};
