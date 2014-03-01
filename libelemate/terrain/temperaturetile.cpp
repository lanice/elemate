#include "temperaturetile.h"

#include <limits>
#include <cmath>

#include <glow/logging.h>

#include <glm/glm.hpp>

#include "basetile.h"

const celsius TemperatureTile::minTemperature = -273.15f;
const celsius TemperatureTile::maxTemperature = std::numeric_limits<celsius>::max();

TemperatureTile::TemperatureTile(Terrain & terrain, const TileID & tileID, const BaseTile & baseTile)
: TerrainTile(terrain, tileID, minTemperature, maxTemperature, 1.0f)
, m_baseTile(baseTile)
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

    static celsius tempStep = .5f;
    static celsius minStep = 0.001f;

    if (m_deltaTime < 0.5)
        return;
    else
        m_deltaTime = 0.0;

    unsigned int minActiveIndex = std::numeric_limits<unsigned int>::max();
    unsigned int maxActiveIndex = std::numeric_limits<unsigned int>::min();
    bool valuesChanged = false;

    for (unsigned int r = 0; r < samplesPerAxis; ++r) {
        unsigned int rowOffset = r*samplesPerAxis;
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
            if (minActiveIndex > index)
                minActiveIndex = index;
            if (maxActiveIndex < index)
                maxActiveIndex = index;
        }
    }
    if (valuesChanged)
        addBufferUpdateRange(minActiveIndex, maxActiveIndex - minActiveIndex + 1);
}
