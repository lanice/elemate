#include "temperaturetile.h"

#include <limits>
#include <cmath>

#include <glow/logging.h>

#include <glm/glm.hpp>

#include "physicaltile.h"

// these values also influent the effect range of the TerrainInteraction (using a std deviation)
const celsius TemperatureTile::minTemperature = -273.15f;
const celsius TemperatureTile::maxTemperature = 1200.0f;    // lava is liquid at temperatures from 700°C to 1200°C

const celsius TemperatureTile::minLavaTemperature = 700.0f;
const celsius TemperatureTile::maxGrassTemperature = 300.0f;

namespace {
    template<typename T>
    struct Bounds {
        Bounds(T min = std::numeric_limits<T>::max(), T max = std::numeric_limits<T>::lowest())
            : min(min), max(max)
        {}
        T min;
        T max;
        void extend(T value)
        {
            if (min > value)
                min = value;
            if (max < value)
                max = value;
        }
    };
}

TemperatureTile::TemperatureTile(Terrain & terrain, const TileID & tileID, PhysicalTile & baseTile, PhysicalTile & liquidTile)
: TerrainTile(terrain, tileID, minTemperature, maxTemperature, 3, 1.0f)
, m_baseTile(baseTile)
, m_liquidTile(liquidTile)
, m_deltaTime(0.0f)
, m_baseBedrockIndex(baseTile.elementIndex("bedrock"))
, m_baseGrassIndex(baseTile.elementIndex("grassland"))
{
    for (unsigned int r = 0; r < samplesPerAxis; ++r) {
        unsigned int rowOffset = r*samplesPerAxis;
        for (unsigned int c = 0; c < samplesPerAxis; ++c) {
            m_values.at(c + rowOffset) = temperatureByHeight(m_baseTile.valueAt(r, c));
        }
    }
}

celsius TemperatureTile::temperatureByHeight(meter height)
{
    static const celsius baseTemp = 20.0f;
    static const celsius baseWaterTemp = 4.0f;
    if (height > 0)
        return (-baseTemp / (m_baseTile.maxValidValue * 0.75f)) * height + baseTemp;
    else
        return ((baseTemp - baseWaterTemp) / m_baseTile.maxValidValue) * height + baseTemp;
}

void TemperatureTile::updatePhysics(double delta)
{
    m_deltaTime += delta;

    if (m_deltaTime < 0.5)
        return;
    else
        m_deltaTime = 0.0;

    for (unsigned int r = 0; r < samplesPerAxis; ++r) {
        unsigned int rowOffset = r*samplesPerAxis;

        Bounds<unsigned int> activeIndex;
        Bounds<unsigned int> activeHeightIndex;
        bool temperaturesChanged = false;
        bool physicalTilesChanged = false;

        for (unsigned int c = 0; c < samplesPerAxis; ++c) {
            const unsigned int index = c + rowOffset;

            // update temperature at current position, continue if it didn't change
            if (!updateTemperature(index))
                continue;

            temperaturesChanged = true;
            activeIndex.extend(index);

            if (updateTerrainType(index)) {
                physicalTilesChanged = true;
                activeHeightIndex.extend(index);
            }

            if (!updateSolidLiquid(index))
                continue;

            physicalTilesChanged = true;
            activeHeightIndex.extend(index);
        }

        if (temperaturesChanged)
            addBufferUpdateRange(activeIndex.min, activeIndex.max - activeIndex.max + 1);
        if (physicalTilesChanged) {
            m_baseTile.addBufferUpdateRange(activeHeightIndex.min, activeHeightIndex.max - activeHeightIndex.min + 1);
            m_liquidTile.addBufferUpdateRange(activeHeightIndex.min, activeHeightIndex.max - activeHeightIndex.min + 1);
        }
    }
}

bool TemperatureTile::updateTemperature(unsigned int index)
{
    static const celsius tempStep = 0.5f;
    static const celsius minStep = 0.001f;

    celsius currentTemp = m_values.at(index);
    celsius reverend = temperatureByHeight(m_baseTile.valueAt(index));
    celsius tempDelta = currentTemp - reverend;

    // ignore small differences
    if (std::abs(tempDelta) < minStep)
        return false;

    if (std::abs(tempDelta) < tempStep)
        m_values.at(index) = reverend;
    else
        m_values.at(index) = currentTemp - glm::sign(tempDelta) * tempStep;

    return true;
}

bool TemperatureTile::updateSolidLiquid(unsigned int index)
{
    // change base terrain / lava terrain heights depending on temperature at current position
    // as the terrain type of the liquid tile depends only on the height for now: ignore heights below 0
    if (m_baseTile.valueAt(index) <= 0.0f)
        return false;

    bool lavaUp = m_values.at(index) >= minLavaTemperature;
    // don't change the heights if they already represent the current temperature
    if (lavaUp == (m_liquidTile.valueAt(index) > m_baseTile.valueAt(index)))
        return false;

    if (lavaUp) {
        m_liquidTile.setValue(index, m_baseTile.valueAt(index));
        m_baseTile.setValue(index, m_baseTile.valueAt(index) - 0.1f);
        m_baseTile.setElement(index, m_baseBedrockIndex);
    }
    else {
        m_baseTile.setValue(index, m_liquidTile.valueAt(index));
        m_liquidTile.setValue(index, m_baseTile.valueAt(index) - 0.1f);
    }

    return true;
}

bool TemperatureTile::updateTerrainType(unsigned int index)
{
    // don't need to remove grass if temperature below the limit
    if (m_values.at(index) < maxGrassTemperature)
        return false;

    // don't need to remove grass that isn't there
    if (m_baseTile.elementIndexAt(index) != m_baseGrassIndex)
        return false;

    // kill the green stuff
    m_baseTile.setElement(index, m_baseBedrockIndex);
    return true;
}
