#pragma once

#include <memory>
#include <string>

#include "terrainsettings.h"

class Terrain;
class TerrainTile;

class TerrainInteractor {
public:
    /** Creates an Interactor for the specified terrain.
      * @param interactMaterial select the material this interactor works with */
    TerrainInteractor(std::shared_ptr<Terrain>& terrain, const std::string & interactMaterial);

    const std::string & interactMaterial() const;
    void setInteractMaterial(const std::string & material);

    /** @return the height of the material this interactor is configured to work with */
    float heightAt(float worldX, float worldZ) const;
    /** @return whether current interact material's layer is the heighest at the world position, meaning the one the player can interact with. */
    bool isHeighestAt(float worldX, float worldZ) const;
    /** Add delta to the current interact material height at a specified world position.
    * The actual height value will be clampted to terrain's [-maxHeight, maxHeight] if necessary.
    * @return the new applied height value or zero if the position is out of range. */
    float changeHeight(float worldX, float worldZ, float delta);
    /** Lowers the terrain at specified coordinate, depending on the given volume and the terrain's resolution
      * This changes the terrain layer holding the interactMaterial this interactor is configure to use */
    void takeOffVolume(float worldX, float worldZ, float volume);

    /** @return the maximum height of all terrain levels at the specified position */
    float terrainHeightAt(float worldX, float worldZ) const;
    /** @return the height of the specified terrain level */
    float levelHeightAt(float worldX, float worldZ, TerrainLevel level) const;

    /** Set the terrain level height at a specified world position to value.
      * @param value the new height value. Will be clamped to terrain's [-maxHeight, maxHeight] if necessary.
      * @return the new applied height value or zero if the position is out of range. */
    float setLevelHeight(float worldX, float worldZ, TerrainLevel level, float value);
    /** Add delta to the terrain level height at a specified world position.
      * The actual height value will be clampted to terrain's [-maxHeight, maxHeight] if necessary.
      * @return the new applied height value or zero if the position is out of range. */
    float changeLevelHeight(float worldX, float worldZ, TerrainLevel level, float delta);

    /** grabs the terrain at worldXZ, storing the current height value */
    float heightGrab(float worldX, float worldZ);
    /** pulls the terrain at worldXZ, setting the height to the grabbed value */
    void heightPull(float worldX, float worldZ);

    std::shared_ptr<const Terrain> terrain() const;
    void setTerrain(std::shared_ptr<Terrain>& terrain);

    static float normalDist(float x, float mean, float stddev);

private:
    std::shared_ptr<Terrain> m_terrain;
    /** the name of the material this interactor currently works on */
    std::string m_interactMaterial;
    /** for internal usage: the terrain level that hold the configured interact material */
    TerrainLevel m_interactLevel;

    /**  */
    float setHeight(TerrainTile & tile, unsigned row, unsigned column, float value);
    void updatePxHeight(const TerrainTile & tile, unsigned minRow, unsigned maxRow, unsigned minColumn, unsigned maxColumn);

    TerrainLevel m_grabbedLevel;
    float m_grabbedHeight;

public:
    TerrainInteractor(TerrainInteractor&) = delete;
    void operator=(TerrainInteractor&) = delete;
    TerrainInteractor() = delete;
};
