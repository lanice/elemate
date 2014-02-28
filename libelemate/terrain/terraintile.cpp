#include "terraintile.h"

#include <cmath>

#include <glow/Texture.h>
#include <glow/Buffer.h>

#include <glm/glm.hpp>

#include "terrain.h"
#include "elements.h"
#include "world.h"

namespace {
    uint32_t calcSamplesPerAxis(uint32_t baseSamples, float scaling) {
        assert(scaling <= 1.0f && scaling > 0.0f);
        if (scaling == 1.0f)
            return baseSamples;

        return static_cast<uint32_t>(std::ceil(baseSamples * scaling));
    }
}

TerrainTile::TerrainTile(Terrain & terrain, const TileID & tileID, float resolutionScaling)
: m_tileID(tileID)
, m_terrain(terrain)
, samplesPerAxis(calcSamplesPerAxis(terrain.settings.maxTileSamplesPerAxis, resolutionScaling))
, resolutionScaling(resolutionScaling)
, samplesPerWorldCoord(samplesPerAxis / terrain.settings.tileBorderLength())
, sampleInterval(terrain.settings.tileBorderLength() / (samplesPerAxis - 1))
, m_isInitialized(false)
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

    m_heightField.resize(samplesPerAxis * samplesPerAxis);
}

TerrainTile::~TerrainTile()
{
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

    assert(m_heightTex);

    m_heightTex->bindActive(GL_TEXTURE0);
}

void TerrainTile::unbind()
{
    m_heightTex->unbindActive(GL_TEXTURE0);
}

void TerrainTile::initialize()
{
    clearBufferUpdateRange();

    m_heightBuffer = new glow::Buffer(GL_TEXTURE_BUFFER);
    m_heightBuffer->setData(m_heightField, GL_DYNAMIC_DRAW);

    m_heightTex = new glow::Texture(GL_TEXTURE_BUFFER);
    m_heightTex->bind();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_heightBuffer->id());
    m_heightBuffer->unbind();

    m_isInitialized = true;
}

float TerrainTile::heightAt(unsigned int row, unsigned int column) const
{
    assert(row < samplesPerAxis && column < samplesPerAxis);
    return m_heightField.at(column + row * samplesPerAxis);
}

bool TerrainTile::heightAt(unsigned int row, unsigned int column, float & height) const
{
    if (row >= samplesPerAxis || column >= samplesPerAxis)
        return false;

    height = heightAt(row, column);
    return true;
}

void TerrainTile::setHeight(unsigned int row, unsigned int column, float value)
{
    assert(row < samplesPerAxis && column < samplesPerAxis);
    assert(std::abs(value) <= m_terrain.settings.maxHeight);
    m_heightField.at(column + row * samplesPerAxis) = value;
}

// mostly from OpenSceneGraph: osgTerrain/Layer
float TerrainTile::interpolatedHeightAt(float normX, float normZ) const
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
    if (mix > 0.0 && heightAt(row, column, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = (rowFraction) * (1.0 - columnFraction);
    if (mix > 0.0 && heightAt(row + 1, column, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = rowFraction * columnFraction;
    if (mix > 0.0 && heightAt(row + 1, column + 1, gridValue))
    {
        value += gridValue * mix;
        div += mix;
    }

    mix = (1.0 - rowFraction) * columnFraction;
    if (mix > 0.0 && heightAt(row, column + 1, gridValue))
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

    float * bufferDest = reinterpret_cast<float*>(m_heightBuffer->mapRange(
        sizeof(float) * m_updateRangeMinMaxIndex.x,
        sizeof(float) * (m_updateRangeMinMaxIndex.y - m_updateRangeMinMaxIndex.x),
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));

    assert(bufferDest);

    unsigned int indexOffset = m_updateRangeMinMaxIndex.x;

    for (const UpdateRange & range : m_bufferUpdateList) {
        assert(indexOffset <= range.startIndex);
        assert(range.startIndex - indexOffset >= 0);
        assert(range.startIndex - indexOffset + range.nbElements < m_heightField.size());
        memcpy(bufferDest + (range.startIndex - indexOffset),
            reinterpret_cast<float*>(m_heightField.data()) + range.startIndex,
            range.nbElements * sizeof(float));
    }

    m_heightBuffer->unmap();

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
