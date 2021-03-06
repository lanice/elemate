#pragma once

#include <string>

#include "terrainsettings.h"

class Terrain;
class TerrainTile;
class LuaWrapper;

class TerrainInteraction
{
public:
    /** Creates an interaction interface for the specified terrain.
      * @param interactElement select the element this instance works with */
    TerrainInteraction(Terrain & terrain, const std::string & interactElement);

    /** Creates an interaction interface for the default terrain.
      * @param interactElement select the element this instance works with */
    TerrainInteraction(const std::string & interactElement);

    static void setDefaultTerrain(Terrain & terrain);

    const std::string & interactElement() const;
    void setInteractElement(const std::string & elementName);

    /** @return name of topmost element at world position */
    const std::string & topmostElementAt(float worldX, float worldZ) const;
    /** @return name of topmost solid element at world position, in the base level */
    const std::string & solidElementAt(float worldX, float worldZ) const;
    /** Fetch the highest element at world position and use its level for interaction functions.
      * @return the name of this element */
    const std::string & useTopmostElementAt(float worldX, float worldZ);
    /** Fetch the highest solid element at world position and use its level for interaction functions.
      * This fetches the element from the base level.
      * @return the name of this element */
    const std::string & useSolidElementAt(float worldX, float worldZ);

    /** @return the height of the element this instance is configured to work with */
    float heightAt(float worldX, float worldZ) const;
    /** @return whether the current interact element's level is the highest at the world position, meaning the one the player can interact with. */
    bool isHeighestAt(float worldX, float worldZ) const;
    /** Add delta to the current interact element height at a specified world position.
    * The actual height value will be clamped to terrain's [-maxHeight, maxHeight] if necessary.
    * @return the new applied height value or zero if the position is out of range. */
    float changeHeight(float worldX, float worldZ, float delta);
    /** Drop some amount of the current interaction element to the world position, so that the level height increases by heightDelta.
      * @return the resulting height at the world position. */
    float dropElement(float worldX, float worldZ, float heightDelta);
    /** If the current interaction element is the topmost at the current world position, gather some amount of it, so that the level height increases by heightDelta.
    * @return the resulting height at the world position. */
    float gatherElement(float worldX, float worldZ, float heightDelta);

    /** @return the maximum height of all terrain levels at the specified position */
    float terrainHeightAt(float worldX, float worldZ) const;
    /** @return the height of the specified terrain level */
    float levelHeightAt(float worldX, float worldZ, TerrainLevel level) const;

    /** Set the terrain level height at a specified world position to value.
      * @param value the new height value. Will be clamped to terrain's [-maxHeight, maxHeight] if necessary.
      * @return the new applied height value or zero if the position is out of range. */
    float setLevelHeight(float worldX, float worldZ, TerrainLevel level, float value, bool setToInteractionElement);
    /** Add delta to the terrain level height at a specified world position.
      * The actual height value will be clamped to terrain's [-maxHeight, maxHeight] if necessary.
      * @return the new applied height value or zero if the position is out of range. */
    float changeLevelHeight(float worldX, float worldZ, TerrainLevel level, float delta, bool setToInteractionElement);

    /** grabs the terrain at worldXZ, storing the current height value */
    float heightGrab(float worldX, float worldZ);
    /** pulls the terrain at worldXZ, setting the height to the grabbed value */
    void heightPull(float worldX, float worldZ);

    const Terrain & terrain() const;

    static float normalDist(float x, float mean, float stddev);

    void registerLuaFunctions(LuaWrapper & lua);

    /** element name if the interaction element is not set and for out for range access */
    static const std::string s_defaultElementName;

private:
    static Terrain * s_defaultTerrain;

    Terrain & m_terrain;
    /** the name of the element this instance currently works on */
    std::string m_interactElement;
    /** for internal usage: the terrain level that hold the configured interact element */
    TerrainLevel m_interactLevel;

    float setValue(TerrainTile & tile, unsigned row, unsigned column, float value, bool setToInteractionElement);

    TerrainLevel m_grabbedLevel;
    float m_grabbedHeight;

public:
    TerrainInteraction(TerrainInteraction&) = delete;
    void operator=(TerrainInteraction&) = delete;
    TerrainInteraction() = delete;
};
