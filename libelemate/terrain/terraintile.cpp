#include "terraintile.h"

#include <cmath>
#include <cstring>

#include <glow/Texture.h>
#include <glow/Buffer.h>

#include <glm/glm.hpp>

#include "terrain.h"
#include "elements.h"
#include "world.h"
#include "texturemanager.h"

namespace {
    uint32_t calcSamplesPerAxis(uint32_t baseSamples, float scaling) {
        assert(scaling <= 1.0f && scaling > 0.0f);
        if (scaling == 1.0f)
            return baseSamples;

        return static_cast<uint32_t>(std::ceil(baseSamples * scaling));
    }
}

TerrainTile::TerrainTile(Terrain & terrain, const TileID & tileID, float minValidValue, float maxValidValue, float resolutionScaling)
: tileName(generateName(tileID))
, m_tileID(tileID)
, m_terrain(terrain)
, samplesPerAxis(calcSamplesPerAxis(terrain.settings.maxTileSamplesPerAxis, resolutionScaling))
, resolutionScaling(resolutionScaling)
, samplesPerWorldCoord(samplesPerAxis / terrain.settings.tileBorderLength())
, sampleInterval(terrain.settings.tileBorderLength() / (samplesPerAxis - 1))
, m_isInitialized(false)
, m_drawHeatMap(false)
, minValidValue(minValidValue)
, maxValidValue(maxValidValue)
{
    terrain.registerTile(tileID, *this);

    // compute position depending on TileID, which sets the row/column positions of the tile
    float minX = terrain.settings.tileBorderLength() * (tileID.x - 0.5f);
    float minZ = terrain.settings.tileBorderLength() * (tileID.z - 0.5f);
    m_transform = glm::mat4(
        sampleInterval, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, sampleInterval, 0,
        minX, 0, minZ, 1);

    clearBufferUpdateRange();

    m_values.resize(samplesPerAxis * samplesPerAxis);
}

TerrainTile::~TerrainTile()
{
}

std::string TerrainTile::generateName(const TileID & tileID)
{
    return "TerrainTile_" + std::to_string(int(tileID.level)) + "_" + std::to_string(tileID.x) + "x" + std::to_string(tileID.z);
}

void TerrainTile::prepareDraw()
{
    if (!m_isInitialized)
        initialize();
    if (!m_bufferUpdateList.empty())
        updateBuffers();
}

void TerrainTile::bind(const CameraEx & /*camera*/)
{
    prepareDraw();

    assert(m_valueTex);
}

void TerrainTile::unbind()
{
}

void TerrainTile::initialize()
{
    clearBufferUpdateRange();

    m_valueBuffer = new glow::Buffer(GL_TEXTURE_BUFFER);
    m_valueBuffer->setData(m_values, GL_DYNAMIC_DRAW);

    m_valueTex = new glow::Texture(GL_TEXTURE_BUFFER);
    m_valueTex->bind();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_valueBuffer->id());
    m_valueBuffer->unbind();

    m_valueTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit(tileName, "values"));
    glActiveTexture(GL_TEXTURE0);
    CheckGLError();


    m_isInitialized = true;
}

void TerrainTile::updatePhysics(double /*delta*/)
{
}

float TerrainTile::valueAt(unsigned int row, unsigned int column) const
{
    assert(row < samplesPerAxis && column < samplesPerAxis);
    return m_values.at(column + row * samplesPerAxis);
}

bool TerrainTile::valueAt(unsigned int row, unsigned int column, float & value) const
{
    if (row >= samplesPerAxis || column >= samplesPerAxis)
        return false;

    value = valueAt(row, column);
    assert(isValueInRange(value));
    return true;
}

void TerrainTile::setValue(unsigned int row, unsigned int column, float value)
{
    assert(row < samplesPerAxis && column < samplesPerAxis);
    assert(isValueInRange(value));
    m_values.at(column + row * samplesPerAxis) = value;
}

// mostly from OpenSceneGraph: osgTerrain/Layer
float TerrainTile::interpolatedValueAt(float normX, float normZ) const
{
    double row_r, column_r;
    double rowFraction = std::modf(normX * samplesPerAxis, &row_r);
    double columnFraction = std::modf(normZ * samplesPerAxis, &column_r);

    unsigned int row = static_cast<unsigned int>(row_r);
    unsigned int column = static_cast<unsigned int>(column_r);

    double value = 0.0;
    double div = 0.0;
    float gridValue;
    double mix;

    mix = (1.0 - rowFraction) * (1.0 - columnFraction);
    if (mix > 0.0 && valueAt(row, column, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = (rowFraction) * (1.0 - columnFraction);
    if (mix > 0.0 && valueAt(row + 1, column, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = rowFraction * columnFraction;
    if (mix > 0.0 && valueAt(row + 1, column + 1, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = (1.0 - rowFraction) * columnFraction;
    if (mix > 0.0 && valueAt(row, column + 1, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    if (div != 0.0)
    {
        return static_cast<float>(value / div);
    }

    return 0.0f;
}

const glm::mat4 & TerrainTile::transform() const
{
    return m_transform;
}

void TerrainTile::updateBuffers()
{
    assert(m_updateRangeMinMaxIndex.x < m_updateRangeMinMaxIndex.y);

    float * bufferDest = reinterpret_cast<float*>(m_valueBuffer->mapRange(
        sizeof(float) * m_updateRangeMinMaxIndex.x,
        sizeof(float) * (m_updateRangeMinMaxIndex.y - m_updateRangeMinMaxIndex.x),
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

    assert(bufferDest);

    unsigned int indexOffset = m_updateRangeMinMaxIndex.x;

    for (const UpdateRange & range : m_bufferUpdateList) {
        assert(indexOffset <= range.startIndex);
        assert(range.startIndex - indexOffset >= 0);
        assert(range.startIndex - indexOffset + range.nbElements <= m_values.size());
        memcpy(bufferDest + (range.startIndex - indexOffset),
            reinterpret_cast<float*>(m_values.data()) + range.startIndex,
            range.nbElements * sizeof(float));
    }

    m_valueBuffer->unmap();

    clearBufferUpdateRange();
}

void TerrainTile::addBufferUpdateRange(unsigned int startIndex, unsigned int nbElements)
{
    if (m_updateRangeMinMaxIndex.x > startIndex)
        m_updateRangeMinMaxIndex.x = startIndex;
    if (m_updateRangeMinMaxIndex.y < startIndex + nbElements)
        m_updateRangeMinMaxIndex.y = startIndex + nbElements;
    m_bufferUpdateList.push_front({ startIndex, nbElements });
}

void TerrainTile::clearBufferUpdateRange()
{
    m_bufferUpdateList.clear();
    m_updateRangeMinMaxIndex = glm::detail::tvec2<unsigned int>(std::numeric_limits<unsigned int>::max(), 0);
}
