#include "terraingenerator.h"

#include <random>
#include <cmath>
#include <cstdint>
#include <limits>
#include <ctime>
#include <functional>
#include <algorithm>

#include <glow/Array.h>
#include <glow/logging.h>

#include "pxcompilerfix.h"
#include <PxRigidStatic.h>
#include <PxPhysics.h>

#include "terrain.h"
#include "basetile.h"
#include "watertile.h"

// Mersenne Twister, preconfigured
namespace {
    std::mt19937 rng;
}

using namespace physx;

namespace {// 1, 3, 8 for 513, 5(look around!) for 1025
    uint32_t seed_val = 5u;
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

std::shared_ptr<Terrain> TerrainGenerator::generate(const World & world) const
{
    std::shared_ptr<Terrain> terrain = std::make_shared<Terrain>(world, m_settings);

    // The tileID determines the position of the current tile in the grid of tiles.
    // Tiles get shifted by -(numTilesPerAxis + 1)/2 so that we have the Tile(0,0,0) in the origin.
    
    int maxxID = m_settings.tilesX - int((m_settings.tilesX + 1) * 0.5);
    int minxID = maxxID - m_settings.tilesX + 1;
    int maxzID = m_settings.tilesZ - int((m_settings.tilesZ + 1) * 0.5);
    int minzID = maxzID - m_settings.tilesZ + 1;

    terrain->minTileXID = minxID;
    terrain->minTileZID = minzID;

    for (int xID = minxID; xID <= maxxID; ++xID)
    for (int zID = minzID; zID <= maxzID; ++zID)
    {
        TileID tileIDBase(TerrainLevel::BaseLevel, xID, zID);

        // create base terrain heightfield with random structure
        glow::FloatArray * baseHeightField = createBasicHeightField(0);
        assert(baseHeightField);

        std::initializer_list<std::string> baseElements = { "bedrock", "sand", "grassland" };

        // create empty heightfield
        unsigned int numSamples = m_settings.rows * m_settings.columns;
        glow::UByteArray * baseTerrainTypeIDs = new glow::UByteArray;
        baseTerrainTypeIDs->resize(numSamples);

        /** create terrain object and pass terrain data */
        BaseTile * baseTile = new BaseTile(*terrain, tileIDBase, baseElements);
        baseTile->setHeightField(*baseHeightField);
        baseTile->m_terrainTypeData = baseTerrainTypeIDs;

        // create the terain using diamond square algorithm
        diamondSquare(*baseTile);
        // and apply the elements to the landscape
        applyElementsByHeight(*baseTile);

        /** same thing for the water lever, just that we do not add a terrain type texture (it consists only of water) */
        TileID tileIDWater(TerrainLevel::WaterLevel, xID, zID);
        glow::FloatArray * waterHeightField = createBasicHeightField(0);
        assert(waterHeightField);
        WaterTile * waterTile = new WaterTile(*terrain, tileIDWater);
        waterTile->setHeightField(*waterHeightField);
        waterTile->m_baseHeightTex = baseTile->m_heightTex;

        /** Create physx objects: an actor with its transformed shapes
          * move tile according to its id, and by one half tile size, so the center of Tile(0,0,0) is in the origin */
        PxTransform pxTerrainTransform = PxTransform(PxVec3(m_settings.tileSizeX() * (xID - 0.5f), 0.0f, m_settings.tileSizeZ() * (zID - 0.5f)));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        terrain->m_pxActors.emplace(tileIDBase, actor);

        baseTile->createPxObjects(*actor);
        waterTile->createPxObjects(*actor);

        waterTile->pxShape()->setFlag(PxShapeFlag::ePARTICLE_DRAIN, true);
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

void TerrainGenerator::diamondSquare(TerrainTile & tile) const
{
    // assuming the edge length of the field is a power of 2, + 1
    // assuming the field is square

    const unsigned fieldEdgeLength = m_settings.rows;
    const float maxHeight = m_settings.maxHeight;

    float randomMax = 50.0f;
    std::function<float(float)> clampHeight = [maxHeight](float value) {
        if (value > maxHeight)
            value = maxHeight;
        if (value < -maxHeight)
            value = -maxHeight;
        return value;
    };

    std::function<void(unsigned int, unsigned int, unsigned int, std::function<float(unsigned int, unsigned int)>&)> squareStep =
        [&tile, fieldEdgeLength, &clampHeight](unsigned int diamondRadius, unsigned int diamondCenterRow, unsigned int diamondCenterColumn, std::function<float(unsigned int, unsigned int)>& heightRnd)
    {
        // get the existing data values first: if we get out of the valid range, wrap around, to the next existing value on the other field side
        int upperRow = signed(diamondCenterRow) - signed(diamondRadius);
        if (upperRow < 0)
            upperRow = fieldEdgeLength - 1 - diamondRadius; // example: nbRows=5, centerRow=0, upperRow gets -1, we want the second last row (with existing value), so it's 3
        int lowerRow = signed(diamondCenterRow) + signed(diamondRadius);
        if (lowerRow >= signed(fieldEdgeLength))
            lowerRow = diamondRadius; // this is easier: use the first row in our column, that is already set
        int leftColumn = signed(diamondCenterColumn) - signed(diamondRadius);
        if (leftColumn < 0)
            leftColumn = fieldEdgeLength - 1 - diamondRadius;
        int rightColumn = signed(diamondCenterColumn) + signed(diamondRadius);
        if (rightColumn >= signed(fieldEdgeLength))
            rightColumn = diamondRadius;
        float value =
            (tile.heightAt(upperRow, diamondCenterColumn)
            + tile.heightAt(lowerRow, diamondCenterColumn)
            + tile.heightAt(diamondCenterRow, leftColumn)
            + tile.heightAt(diamondCenterRow, rightColumn))
            * 0.25f
            + heightRnd(diamondCenterRow, diamondCenterColumn);
        tile.setHeight(diamondCenterRow, diamondCenterColumn, clampHeight(value));

        // in case we are at the borders of the tile: also set the value at the opposite border, to allow seamless tile wrapping
        if (upperRow > signed(diamondCenterRow))
            tile.setHeight(fieldEdgeLength - 1, diamondCenterColumn, value);
        if (leftColumn > signed(diamondCenterColumn))
            tile.setHeight(diamondCenterRow, fieldEdgeLength - 1, value);
    };
    
    unsigned nbSquareRows = 1; // number of squares in a row, doubles each time the current edge length increases [same for the columns]

    for (unsigned int len = fieldEdgeLength; len > 2; len = (len / 2) + 1) // legnth: 9, 5, 3, finished
    {
        const unsigned int currentEdgeLength = len;
        std::uniform_real_distribution<float> dist(-randomMax, randomMax);
        std::function<float(unsigned int, unsigned int)> heightRndPos =
            [fieldEdgeLength, &dist](unsigned int row, unsigned int column) {
            glm::vec2 pos(row, column);
            pos = pos / (fieldEdgeLength - 1.0f) * 2.0f - 1.0f;
            return float(glm::length(pos)) * dist(rng);
            //return std::abs(float(row + column) / float(2 * fieldEdgeLength - 2) * 2.0f - 1.0f) * dist(rng);
        };
        // create diamonds
        for (unsigned int rowN = 0; rowN < nbSquareRows; ++rowN) {
            const unsigned int row = rowN * (currentEdgeLength - 1);
            const unsigned int midpointRow = row + (currentEdgeLength - 1) / 2; // this is always divisible, because of the edge length 2^n + 1
            for (unsigned int columnN = 0; columnN < nbSquareRows; ++columnN) {
                const unsigned int column = columnN * (currentEdgeLength - 1);
                //const unsigned int index = column + rowOffset;
                const unsigned int midpointColumn = column + (currentEdgeLength - 1) / 2;
                float heightValue =
                    (tile.heightAt(row, column)
                    + tile.heightAt(row + currentEdgeLength - 1, column)
                    + tile.heightAt(row, column + currentEdgeLength - 1)
                    + tile.heightAt(row + currentEdgeLength - 1, column + currentEdgeLength - 1))
                    * 0.25f
                    + heightRndPos(midpointRow, midpointColumn);

                tile.setHeight(midpointRow, midpointColumn, clampHeight(heightValue));
            }
        }

        // create squares
        unsigned int diamondRadius = (currentEdgeLength - 1) / 2;
        // don't iterate over the last row/column here. These values are set with the first row/column, to allow seamless tile wrapping
        for (unsigned int rowN = 0; rowN < nbSquareRows; ++rowN) {
            const unsigned int seedpointRow = rowN * (currentEdgeLength - 1);
            for (unsigned int columnN = 0; columnN < nbSquareRows; ++columnN) {
                const unsigned int seedpointColumn = columnN * (currentEdgeLength - 1);

                unsigned int rightDiamondColumn = seedpointColumn + currentEdgeLength / 2;
                if (rightDiamondColumn < m_settings.columns)
                    squareStep(diamondRadius, seedpointRow, rightDiamondColumn, heightRndPos);

                unsigned int bottomDiamondRow = seedpointRow + currentEdgeLength / 2;
                if (bottomDiamondRow < m_settings.rows)
                    squareStep(diamondRadius, bottomDiamondRow, seedpointColumn, heightRndPos);
            }
        }

        nbSquareRows *= 2;
        randomMax *= 0.5;
    }
}

void TerrainGenerator::applyElementsByHeight(BaseTile & tile) const
{
    uint8_t sand = tile.elementIndex("sand");
    uint8_t grassland = tile.elementIndex("grassland");
    uint8_t bedrock = tile.elementIndex("bedrock");

    float sandMaxHeight = 2.5f;     // under water + shore
    float grasslandMaxHeight = m_settings.maxHeight * 0.2f;

    for (unsigned int row = 0; row < m_settings.rows - 1; ++row) {
        for (unsigned int column = 0; column < m_settings.columns - 1; ++column) {
            float height = 0.25f * (
                tile.heightAt(row, column)
                + tile.heightAt(row + 1, column)
                + tile.heightAt(row, column + 1)
                + tile.heightAt(row + 1, column + 1));
            if (height < sandMaxHeight) {
                tile.setElement(row, column, sand);
                continue;
            }
            if (height < grasslandMaxHeight) {
                tile.setElement(row, column, grassland);
                continue;
            }
            tile.setElement(row, column, bedrock);
        }
    }
}

glow::UByteArray * TerrainGenerator::gougeRiverBed(glow::FloatArray & heightField, const std::initializer_list<std::string> & baseElements) const
{
    std::function<float(float, float)> riverCourse = [](float normRow, float normColumn)
    {
        return std::abs(5.0f * std::pow(normRow, 3) - normColumn);
    };

    const uint8_t bedrockIndex = static_cast<uint8_t>(std::find(baseElements.begin(), baseElements.end(), "bedrock") - baseElements.begin());
    const uint8_t sandIndex = static_cast<uint8_t>(std::find(baseElements.begin(), baseElements.end(), "sand") - baseElements.begin());

    const float riverScale = 0.15f;
    const float normXScale = 1.0f;//m_settings.tileSizeX() / 30.0f;
    const float normZScale = 1.0f;//m_settings.tileSizeZ() / 30.0f;

    unsigned int numSamples = m_settings.rows * m_settings.columns;

    glow::UByteArray * terrainTypeIDs = new glow::UByteArray;
    terrainTypeIDs->resize(numSamples);

    for (unsigned row = 0; row < m_settings.rows; ++row)
    {
        unsigned rowOffset = row * m_settings.columns;
        float normalizedRow = float(row) / m_settings.rows * normZScale;
        for (unsigned column = 0; column < m_settings.columns; ++column)
        {
            float normalizedColumn = float(column) / m_settings.columns * normXScale;
            unsigned index = column + rowOffset;
            float value = riverCourse(normalizedRow, normalizedColumn);
            if (value > riverScale)
                value = riverScale;
            value -= riverScale * 0.5f;
            value -= 0.5f*maxBasicHeightVariance();
            value *= m_settings.maxHeight;
            heightField.at(index) += value;
            if (heightField.at(index) <= 0) {
                terrainTypeIDs->at(index) = sandIndex;
            }
            else {
                terrainTypeIDs->at(index) = bedrockIndex;
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
    unsigned int xSamplesui = static_cast<unsigned int>((ceil(m_settings.sizeX * xzSamplesPerCoord)));
    m_settings.rows = xSamplesui >= 2 ? xSamplesui : 2;
    unsigned int zSamplesui = static_cast<unsigned int>((ceil(m_settings.sizeZ * xzSamplesPerCoord)));
    m_settings.columns = zSamplesui >= 2 ? zSamplesui : 2;
}

float TerrainGenerator::samplesPerWorldCoord() const
{
    return m_settings.samplesPerWorldCoord();
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
