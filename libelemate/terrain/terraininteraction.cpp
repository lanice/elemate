#include "terraininteraction.h"

#include <algorithm>
#include <cmath>

#include <glow/logging.h>

#include "utils/pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxScene.h>
#include <PxShape.h>
#include <PxRigidStatic.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#ifdef PX_WINDOWS
#include <gpu/PxParticleGpu.h>
#endif

#include "terrain.h"
#include "terraintile.h"
#include "physicswrapper.h"
#include "lua/luawrapper.h"

using namespace physx;

const std::string TerrainInteraction::s_defaultElementName = "default";

float TerrainInteraction::normalDist(float x, float mean, float stddev)
{
    return  // n(x) = n(x) = 1 / (stddev*sqrt(2*pi)) * exp( - (x-mean)^2 / (2*stddev^2))
        1.0f / (stddev * std::sqrt(2.0f * glm::pi<float>()))
        * std::exp(-(x - mean) * (x - mean) / (2.0f * stddev * stddev));
}

TerrainInteraction::TerrainInteraction(std::shared_ptr<Terrain>& terrain, const std::string & interactElement)
: m_terrain(terrain)
, m_interactElement(interactElement)
, m_interactLevel(levelForElement(interactElement))
{
}

const std::string & TerrainInteraction::interactElement() const
{
    return m_interactElement;
}

void TerrainInteraction::setInteractElement(const std::string & elementName)
{
    m_interactElement = elementName;
    m_interactLevel = levelForElement(elementName);
}

const std::string & TerrainInteraction::topmostElementAt(float worldX, float worldZ) const
{
    TerrainLevel topmostLevel = m_terrain->heighestLevelAt(worldX, worldZ);

    std::shared_ptr<TerrainTile> tile = nullptr;
    unsigned int row, column;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, topmostLevel, tile, row, column))
        return s_defaultElementName;

    assert(tile);

    return tile->elementAt(row, column);
}

const std::string & TerrainInteraction::useTopmostElementAt(float worldX, float worldZ)
{
    setInteractElement(topmostElementAt(worldX, worldZ));
    return m_interactElement;
}

const std::string & TerrainInteraction::solidElementAt(float worldX, float worldZ) const
{
    std::shared_ptr<TerrainTile> tile = nullptr;
    unsigned int row, column;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, TerrainLevel::BaseLevel, tile, row, column))
        return s_defaultElementName;

    assert(tile);

    return tile->elementAt(row, column);
}

const std::string & TerrainInteraction::useSolidElementAt(float worldX, float worldZ)
{
    setInteractElement(solidElementAt(worldX, worldZ));
    return m_interactElement;
}

float TerrainInteraction::heightAt(float worldX, float worldZ) const
{
    return m_terrain->heightAt(worldX, worldZ, m_interactLevel);
}

bool TerrainInteraction::isHeighestAt(float worldX, float worldZ) const
{
    if (TerrainLevels.size() == 1)
        return true;

    float othersMaxHeight = std::numeric_limits<float>::lowest();
    for (TerrainLevel level : TerrainLevels) {
        if (level == m_interactLevel)
            continue;
        othersMaxHeight = std::max(m_terrain->heightAt(worldX, worldZ, level), othersMaxHeight);
    }
    float myHeight = m_terrain->heightAt(worldX, worldZ, m_interactLevel);
    return othersMaxHeight < myHeight;
}

float TerrainInteraction::dropElement(float worldX, float worldZ, float heightDelta)
{
    assert(heightDelta > 0);
    return changeLevelHeight(worldX, worldZ, m_interactLevel, heightDelta, true);
}

float TerrainInteraction::gatherElement(float worldX, float worldZ, float heightDelta)
{
    assert(heightDelta > 0);
    return changeLevelHeight(worldX, worldZ, m_interactLevel, -heightDelta, false);
}

float TerrainInteraction::changeHeight(float worldX, float worldZ, float delta)
{
    return changeLevelHeight(worldX, worldZ, m_interactLevel, delta, false);
}

float TerrainInteraction::terrainHeightAt(float worldX, float worldZ) const
{
    return m_terrain->heightTotalAt(worldX, worldZ);
}

float TerrainInteraction::levelHeightAt(float worldX, float worldZ, TerrainLevel level) const
{
    return m_terrain->heightAt(worldX, worldZ, level);
}

float TerrainInteraction::setLevelHeight(float worldX, float worldZ, TerrainLevel level, float value, bool setToInteractionElement)
{
    std::shared_ptr<TerrainTile> tile = nullptr;
    unsigned int row, column;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, row, column))
        return 0.0f;

    assert(tile);

    return setHeight(*tile.get(), row, column, value, setToInteractionElement);
}

float TerrainInteraction::changeLevelHeight(float worldX, float worldZ, TerrainLevel level, float delta, bool setToInteractionElement)
{
    std::shared_ptr<TerrainTile> tile;
    unsigned int row, column;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, row, column))
        return 0.0f;

    assert(tile);

    float height = tile->heightAt(row, column);

    return setHeight(*tile.get(), row, column, height + delta, setToInteractionElement);
}

float TerrainInteraction::setHeight(TerrainTile & tile, unsigned row, unsigned column, float value, bool setToInteractionElement)
{
    float stddev = 7.0f; // TODO: script this, element specific value
    assert(stddev > 0);

    const TerrainSettings & settings = m_terrain->settings;

    /** clamp height value */
    if (value < -settings.maxHeight) value = -settings.maxHeight;
    if (value > settings.maxHeight) value = settings.maxHeight;

    // define the size of the affected interaction area, in grid coords
    const float effectRadiusWorld = stddev * 3;
    const uint32_t effectRadius = static_cast<uint32_t>(std::ceil(effectRadiusWorld * settings.samplesPerWorldCoord())); // = 0 means to change only the value at (row,column)

    bool moveUp = (value - tile.heightAt(row, column)) > 0;
    int invert = moveUp ? 1 : -1;   // invert the curve if moving downwards

    float norm0 = normalDist(0, 0, stddev);
    float maxHeight = settings.maxHeight;
    std::function<float(float)> interactHeight = [stddev, norm0, value, maxHeight, invert] (float x) {
        return normalDist(x, 0, stddev)     // - normalize normDist to
                   / norm0                  //    normDist value at interaction center
               * (2 * maxHeight + 10)       // - scale to height range + 10 to omit norm values near 0
               * invert                     // - mirror the curve along the y axis if moving downward
               + value                      // - move along y so that value==0 => y==0
               - (2*maxHeight + 10) * invert;
    };

    unsigned int minRow, maxRow, minColumn, maxColumn;
    {
        // unchecked signed min/max values, possibly < 0 or > numRows/Column
        int iMinRow = row - effectRadius, iMaxRow = row + effectRadius, iMinColumn = column - effectRadius, iMaxColumn = column + effectRadius;
        // work on rows and column that are in range of the terrain tile settings and larger than 0
        minRow = iMinRow < 0 ? 0 : (iMinRow >= static_cast<signed>(settings.rows) ? settings.rows - 1 : static_cast<unsigned int>(iMinRow));
        maxRow = iMaxRow < 0 ? 0 : (iMaxRow >= static_cast<signed>(settings.rows) ? settings.rows - 1 : static_cast<unsigned int>(iMaxRow));
        minColumn = iMinColumn < 0 ? 0 : (iMinColumn >= static_cast<signed>(settings.columns) ? settings.columns - 1 : static_cast<unsigned int>(iMinColumn));
        maxColumn = iMaxColumn < 0 ? 0 : (iMaxColumn >= static_cast<signed>(settings.columns) ? settings.columns - 1 : static_cast<unsigned int>(iMaxColumn));
    }

    uint8_t elementIndex = tile.elementIndex(m_interactElement);

    for (unsigned int r = minRow; r <= maxRow; ++r) {
        float relWorldX = (signed(r) - signed(row)) * settings.sampleInterval();
        for (unsigned int c = minColumn; c <= maxColumn; ++c) {
            float relWorldZ = (signed(c) - signed(column)) * settings.sampleInterval();

            float localRadius = std::sqrt(relWorldX*relWorldX + relWorldZ*relWorldZ);

            if (localRadius > effectRadiusWorld)  // interaction in a circle, not square
                continue;

            float newLocalHeight = interactHeight(localRadius);

            bool localMoveUp = newLocalHeight > tile.heightAt(r, c);
            // don't do anything if we pull up the terrain but the local heightpoint is already higher than its calculated height. (vice versa)
            if (localMoveUp != moveUp)
                continue;

            tile.setHeight(r, c, newLocalHeight);
            if (setToInteractionElement)
                tile.setElement(r, c, elementIndex);
        }
        tile.addBufferUpdateRange(minColumn + r * settings.columns, 1u + effectRadius * 2u);
    }

    tile.addToPxUpdateBox(minRow, maxRow, minColumn, maxColumn);

    return value;
}

float TerrainInteraction::heightGrab(float worldX, float worldZ)
{
    setInteractElement(solidElementAt(worldX, worldZ));
    m_grabbedLevel = m_interactLevel;
    return m_grabbedHeight = m_terrain->heightAt(worldX, worldZ, m_interactLevel);
}

void TerrainInteraction::heightPull(float worldX, float worldZ)
{
    setLevelHeight(worldX, worldZ, m_grabbedLevel, m_grabbedHeight, true);
}

std::shared_ptr<const Terrain> TerrainInteraction::terrain() const
{
    return m_terrain;
}

void TerrainInteraction::setTerrain(std::shared_ptr<Terrain>& terrain)
{
    m_terrain = terrain;
}

void TerrainInteraction::registerLuaFunctions(LuaWrapper * lua)
{
    std::function<float(float, float)> func0 = [=](float worldX, float worldZ)
    { return heightAt(worldX, worldZ); };

    std::function<bool(float, float)> func1 = [=](float worldX, float worldZ)
    { return isHeighestAt(worldX, worldZ); };

    std::function<float(float, float, float)> func2 = [=](float worldX, float worldZ, float delta)
    { return changeHeight(worldX, worldZ, delta); };

    std::function<float(float, float, float)> func3 = [=](float worldX, float worldZ, float heightDelta)
    { return dropElement(worldX, worldZ, heightDelta); };

    std::function<float(float, float, float)> func4 = [=](float worldX, float worldZ, float heightDelta)
    { return gatherElement(worldX, worldZ, heightDelta); };

    std::function<float(float, float)> func5 = [=](float worldX, float worldZ)
    { return terrainHeightAt(worldX, worldZ); };

    std::function<float(float, float)> func6 = [=](float worldX, float worldZ)
    { return heightGrab(worldX, worldZ); };


    std::function<int(std::string)> func7 = [=](std::string elementName)
    { setInteractElement(elementName); return 0; };

    lua->Register("terrain_heightAt", func0);
    lua->Register("terrain_isHeighestAt", func1);
    lua->Register("terrain_changeHeight", func2);
    lua->Register("terrain_dropElement", func3);
    lua->Register("terrain_gatherElement", func4);
    lua->Register("terrain_terrainHeightAt", func5);
    lua->Register("terrain_heightGrab", func6);
    lua->Register("setInteractElement", func7);
}
