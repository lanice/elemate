#pragma once

#include <memory>

#include "elemateterrain.h"

class ElemateHeightFieldTerrain;

class TerrainInteractor {
public:
    TerrainInteractor(std::shared_ptr<ElemateHeightFieldTerrain>& terrain);

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

    std::shared_ptr<ElemateHeightFieldTerrain> terrain() const;
    void setTerrain(std::shared_ptr<ElemateHeightFieldTerrain>& terrain);

private:
    std::shared_ptr<ElemateHeightFieldTerrain> m_terrain;

    float setHeight(osgTerrain::TerrainTile & tile, unsigned physxRow, unsigned physxColumn, float value);
    void setPxHeight(const osgTerrain::TileID & tileID, unsigned physxRow, unsigned physxColumn, float value);

    TerrainLevel m_grabbedLevel;
    float m_grabbedHeight;
};
