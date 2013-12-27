#include "terraingenerator.h"

#include <random>
#include <cstdint>
#include <limits>
#include <ctime>
#include <functional>

#include <glow/Array.h>
#include <glow/logging.h>

#include <PxRigidStatic.h>
#include <PxShape.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>

#include "terrain.h"
#include "basetile.h"
#include "watertile.h"
#include "elements.h"

// Mersenne Twister, preconfigured
// keep one global instance, !per thread!

std::mt19937 rng;

using namespace physx;

namespace {
    uint32_t seed_val = static_cast<uint32_t>(std::time(0));
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

std::shared_ptr<Terrain> TerrainGenerator::generate() const
{
    std::shared_ptr<Terrain> terrain = std::make_shared<Terrain>(m_settings);

    /** The tileID determines the position of the current tile in the grid of tiles.
      * Tiles get shifted by -(numTilesPerAxis + 1)/2 so that we have the Tile(0,0,0) in the origin.
      */
    
    int maxxID = m_settings.tilesX - int((m_settings.tilesX + 1) * 0.5);
    int minxID = maxxID - m_settings.tilesX + 1;
    int maxzID = m_settings.tilesZ - int((m_settings.tilesZ + 1) * 0.5);
    int minzID = maxzID - m_settings.tilesZ + 1;

    terrain->minTileXID = minxID;
    terrain->minTileZID = minzID;
    
    assert(m_settings.tilesX == 1 && 1 == m_settings.tilesZ);

    for (int xID = minxID; xID <= maxxID; ++xID)
    for (int zID = minzID; zID <= maxzID; ++zID)
    {
        /** 1. Create physx terrain heightfield data.
            2. Add some landscape to it (change heightfield, add material information)
            3. Copy physx data to osg and create osg terrain tile.
            4. Create physx actor and shape.
            5. Set some uniforms, valid for the whole terrain. */

        TileID tileIDBase(TerrainLevel::BaseLevel, xID, zID);
        //PxHeightFieldSample * pxBaseHeightFieldSamples
        glow::FloatArray * baseHeightField = createBasicHeightField(m_settings.maxBasicHeightVariance);
        assert(baseHeightField);

        glow::UByteArray * baseTerrainTypeIDs = gougeRiverBed(*baseHeightField);

        /** create terrain object and pass terrain data */
        BaseTile * baseTile = new BaseTile(*terrain, tileIDBase);
        baseTile->m_heightField = baseHeightField;
        baseTile->m_terrainTypeData = baseTerrainTypeIDs;

        /** same thing for the water lever, just that we do not add a terrain type texture (it consists only of water) */
        TileID tileIDWater(TerrainLevel::WaterLevel, xID, zID);
        glow::FloatArray * waterHeightField = createBasicHeightField(0);
        assert(waterHeightField);
        WaterTile * waterTile = new WaterTile(*terrain, tileIDWater);
        waterTile->m_heightField = waterHeightField;

        /** Create physx objects: an actor with its transformed shapes
          * move tile according to its id, and by one half tile size, so the center of Tile(0,0,0) is in the origin */
        PxTransform pxTerrainTransform = PxTransform(PxVec3(m_settings.tileSizeX() * (xID - 0.5f), 0.0f, m_settings.tileSizeZ() * (zID - 0.5f)));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        terrain->m_pxActors.emplace(tileIDBase, actor);

        baseTile->createPxObjects(*actor);
        waterTile->createPxObjects(*actor);
    }

    return terrain;
}

glow::FloatArray * TerrainGenerator::createBasicHeightField(float maxHeightVariance) const
{
    assert(m_settings.rows >= 2);
    assert(m_settings.columns >= 2);
    assert(m_settings.maxHeight > 0);
    assert(maxHeightVariance >= 0);
    assert(m_settings.maxHeight >= maxHeightVariance);

    float partHeight = maxHeightVariance;
    std::uniform_real_distribution<float> uniform_dist(-partHeight, partHeight);

    std::function<float()> getHeight;
    if (maxHeightVariance == 0)
        getHeight = [] () {return 0.0f; };
    else
        getHeight = std::bind(uniform_dist, rng);

    unsigned int numSamples = m_settings.rows * m_settings.columns;

    // physx stores values in row major order (means starting with all values (per column) for the first row)
    glow::FloatArray * heightField = new glow::FloatArray;
    heightField->resize(numSamples);
    for (unsigned i = 0; i < numSamples; ++i) {
        heightField->at(i) = getHeight();
    }

    return heightField;
}

glow::UByteArray * TerrainGenerator::gougeRiverBed(glow::FloatArray & heightField) const
{
    std::function<float(float, float)> riverCourse = [](float normRow, float normColumn)
    {
        return abs(5 * powf(normRow, 3.0) - normColumn);
    };

    static const float riverScale = 0.15f;

    unsigned int numSamples = m_settings.rows * m_settings.columns;

    glow::UByteArray * terrainTypeIDs = new glow::UByteArray;
    terrainTypeIDs->resize(numSamples);

    for (unsigned row = 0; row < m_settings.rows; ++row)
    {
        unsigned rowOffset = row * m_settings.columns;
        float normalizedRow = float(row) / m_settings.rows;
        for (unsigned column = 0; column < m_settings.columns; ++column)
        {
            float normalizedColumn = float(column) / m_settings.columns;
            unsigned index = column + rowOffset;
            float value = riverCourse(normalizedRow, normalizedColumn);
            if (value > riverScale)
                value = riverScale;
            value -= riverScale * 0.5f;
            value -= 0.5f*maxBasicHeightVariance();
            value *= m_settings.maxHeight;
            heightField.at(index) += value;
            if (heightField.at(index) <= 0) {
                terrainTypeIDs->at(index) = 2;  // this is dirt
            }
            else {
                terrainTypeIDs->at(index) = 1;  // this is bedrock
            }
        }
    }

    return terrainTypeIDs;
}

void TerrainGenerator::setExtentsInWorld(float x, float z)
{
    assert(x > 0 && z > 0);
    m_settings.sizeX = x;
    m_settings.sizeZ = z;
}

float TerrainGenerator::xExtens() const
{
    return m_settings.sizeX;
}

float TerrainGenerator::zExtens() const
{
    return m_settings.sizeZ;
}

void TerrainGenerator::applySamplesPerWorldCoord(float xzSamplesPerCoord)
{
    assert(xzSamplesPerCoord > 0.0f);
    unsigned int xSamplesui = unsigned int(ceil(m_settings.sizeX * xzSamplesPerCoord));
    m_settings.rows = xSamplesui >= 2 ? xSamplesui : 2;
    unsigned int zSamplesui = unsigned int(ceil(m_settings.sizeZ * xzSamplesPerCoord));
    m_settings.columns = zSamplesui >= 2 ? zSamplesui : 2;
}

float TerrainGenerator::samplesPerWorldXCoord() const
{
    return m_settings.samplesPerXCoord();
}

float TerrainGenerator::samplesPerWorldZCoord() const
{
    return m_settings.samplesPerZCoord();
}

void TerrainGenerator::setTilesPerAxis(unsigned x, unsigned z)
{
    assert(x >= 1 && z >= 1);
    m_settings.tilesX = x;
    m_settings.tilesZ = z;
}

int TerrainGenerator::tilesPerXAxis() const
{
    return m_settings.tilesX;
}

int TerrainGenerator::tilesPerZAxis() const
{
    return m_settings.tilesZ;
}

void TerrainGenerator::setMaxHeight(float height)
{
    assert(height > 0.0f);
    m_settings.maxHeight = height;
}

float TerrainGenerator::maxHeight() const
{
    return m_settings.maxHeight;
}

void TerrainGenerator::setMaxBasicHeightVariance(float variance)
{
    assert(variance >= 0.0f);
    m_settings.maxBasicHeightVariance = variance;
}

float TerrainGenerator::maxBasicHeightVariance() const
{
    return m_settings.maxBasicHeightVariance;
}
