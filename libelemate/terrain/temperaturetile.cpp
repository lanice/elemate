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

    static celsius tempStep = 0.5f;
    static celsius minStep = 0.001f;

    if (m_deltaTime < 0.5)
        return;
    else
        m_deltaTime = 0.0;

    for (unsigned int r = 0; r < samplesPerAxis; ++r) {
        unsigned int rowOffset = r*samplesPerAxis;

        Bounds<unsigned int> activeIndex;
        Bounds<unsigned int> activeHeightIndex;
        bool valuesChanged = false;
        bool heightsChanged = false;

        for (unsigned int c = 0; c < samplesPerAxis; ++c) {
            const unsigned int index = c + rowOffset;

            celsius currentTemp = m_values.at(c + rowOffset);
            celsius reverend = temperatureByHeight(m_baseTile.valueAt(r, c));
            celsius tempDelta = currentTemp - reverend;

            // ignore small differences
            if (std::abs(tempDelta) < minStep)
                continue;

            if (std::abs(tempDelta) < tempStep)
                m_values.at(index) = reverend;
            else
                m_values.at(index) = currentTemp - glm::sign(tempDelta) * tempStep;

            valuesChanged = true;
            activeIndex.extend(index);

            // change base terrain / lava terrain heights depending on temperature at current position
            // as the terrain type of the liquid tile depends only on the height for now: ignore heights below 0
            if (m_baseTile.valueAt(r, c) <= 0.0f)
                continue;
            bool lavaUp = m_values.at(index) >= minLavaTemperature;
            // don't change the heights if they already represent the current temperature
            if (lavaUp == (m_liquidTile.valueAt(r, c) > m_baseTile.valueAt(r, c)))
                continue;
            
            heightsChanged = true;
            if (lavaUp) {
                m_liquidTile.setValue(r, c, m_baseTile.valueAt(r, c));
                m_baseTile.setValue(r, c, m_baseTile.valueAt(r, c) - 0.1f);
            }
            else {
                m_baseTile.setValue(r, c, m_liquidTile.valueAt(r, c));
                m_liquidTile.setValue(r, c, m_baseTile.valueAt(r, c) - 0.1f);
            }
            activeHeightIndex.extend(index);
        }

        if (valuesChanged)
            addBufferUpdateRange(activeIndex.min, activeIndex.max - activeIndex.max + 1);
        if (heightsChanged) {
            m_baseTile.addBufferUpdateRange(activeHeightIndex.min, activeHeightIndex.max - activeHeightIndex.min + 1);
            m_liquidTile.addBufferUpdateRange(activeHeightIndex.min, activeHeightIndex.max - activeHeightIndex.min + 1);

            m_baseTile.addToPxUpdateBox(0, 0, 0, 0);
            m_liquidTile.addToPxUpdateBox(0, 0, 0, 0);
        }
    }
}
