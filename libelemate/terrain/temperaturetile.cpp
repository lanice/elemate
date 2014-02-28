#include "temperaturetile.h"

#include <limits>
#include <cmath>

const float TemperatureTile::minTemperature = -273.15f;
const float TemperatureTile::maxTemperature = std::numeric_limits<float>::max();

TemperatureTile::TemperatureTile(Terrain & terrain, const TileID & tileID)
: TerrainTile(terrain, tileID, minTemperature, maxTemperature, 1.0f)
{
    for (unsigned int r = 0; r < samplesPerAxis; ++r) {
        unsigned int rowOffset = r*samplesPerAxis;
        for (unsigned int c = 0; c < samplesPerAxis; ++c) {
            
            float value = (std::sin(0.5f * r) + std::sin(0.5f * c)) *  5.0f;
            m_values.at(c + rowOffset) = value;
        }
    }
}
