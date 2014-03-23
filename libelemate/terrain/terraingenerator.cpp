#include "terraingenerator.h"

#include <random>
#include <cmath>
#include <ctime>
#include <functional>
#include <algorithm>

#include <glow/logging.h>

#include "utils/pxcompilerfix.h"
#include <PxRigidStatic.h>
#include <PxPhysics.h>
#include <PxScene.h>

#include "terrain.h"
#include "basetile.h"
#include "liquidtile.h"
#include "temperaturetile.h"

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

std::shared_ptr<Terrain> TerrainGenerator::generate() const
{
    std::shared_ptr<Terrain> terrain = std::make_shared<Terrain>(m_settings);

    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScene;
    PxGetPhysics().getScenes(&pxScene, 1);

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

        std::initializer_list<std::string> baseElements = { "bedrock", "sand", "grassland" };

        /** create terrain object and pass terrain data */
        BaseTile * baseTile = new BaseTile(*terrain, tileIDBase, baseElements);

        // create the terrain using diamond square algorithm
        diamondSquare(*baseTile);
        // and apply the elements to the landscape
        applyElementsByHeight(*baseTile);

        /** same thing for the liquid level, just that we do not add a terrain type texture */
        TileID tileIDLiquid(TerrainLevel::WaterLevel, xID, zID);
        LiquidTile * liquidTile = new LiquidTile(*terrain, tileIDLiquid);

        /** Create physx objects: an actor with its transformed shapes
          * move tile according to its id, and by one half tile size, so the center of Tile(0,0,0) is in the origin */
        PxTransform pxTerrainTransform = PxTransform(PxVec3(m_settings.tileBorderLength() * (xID - 0.5f), 0.0f, m_settings.tileBorderLength() * (zID - 0.5f)));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        terrain->m_pxActors.emplace(tileIDBase, actor);

        baseTile->createPxObjects(*actor);
        liquidTile->createPxObjects(*actor);

        pxScene->addActor(*actor);

        TileID temperatureID(TerrainLevel::TemperatureLevel, xID, zID);
        // the tile registers itself in the terrain
        new TemperatureTile(*terrain, temperatureID, *baseTile, *liquidTile);
    }

    return terrain;
}

void TerrainGenerator::diamondSquare(TerrainTile & tile) const
{
    // assuming the edge length of the field is a power of 2, + 1
    // assuming the field is square

    const unsigned fieldEdgeLength = tile.samplesPerAxis;
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
            (tile.valueAt(upperRow, diamondCenterColumn)
            + tile.valueAt(lowerRow, diamondCenterColumn)
            + tile.valueAt(diamondCenterRow, leftColumn)
            + tile.valueAt(diamondCenterRow, rightColumn))
            * 0.25f
            + heightRnd(diamondCenterRow, diamondCenterColumn);

        float clampedHeight = clampHeight(value);
        tile.setValue(diamondCenterRow, diamondCenterColumn, clampedHeight);

        // in case we are at the borders of the tile: also set the value at the opposite border, to allow seamless tile wrapping
        if (upperRow > signed(diamondCenterRow))
            tile.setValue(fieldEdgeLength - 1, diamondCenterColumn, clampedHeight);
        if (leftColumn > signed(diamondCenterColumn))
            tile.setValue(diamondCenterRow, fieldEdgeLength - 1, clampedHeight);
    };
    
    unsigned nbSquareRows = 1; // number of squares in a row, doubles each time the current edge length increases [same for the columns]

    for (unsigned int len = fieldEdgeLength; len > 2; len = (len / 2) + 1) // length: 9, 5, 3, finished
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
                const unsigned int midpointColumn = column + (currentEdgeLength - 1) / 2;
                float heightValue =
                    (tile.valueAt(row, column)
                    + tile.valueAt(row + currentEdgeLength - 1, column)
                    + tile.valueAt(row, column + currentEdgeLength - 1)
                    + tile.valueAt(row + currentEdgeLength - 1, column + currentEdgeLength - 1))
                    * 0.25f
                    + heightRndPos(midpointRow, midpointColumn);

                tile.setValue(midpointRow, midpointColumn, clampHeight(heightValue));
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
                if (rightDiamondColumn < tile.samplesPerAxis)
                    squareStep(diamondRadius, seedpointRow, rightDiamondColumn, heightRndPos);

                unsigned int bottomDiamondRow = seedpointRow + currentEdgeLength / 2;
                if (bottomDiamondRow < tile.samplesPerAxis)
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

    for (unsigned int row = 0; row < tile.samplesPerAxis - 1; ++row) {
        const unsigned int rowOffset = row * tile.samplesPerAxis;
        for (unsigned int column = 0; column < tile.samplesPerAxis - 1; ++column) {
            const unsigned int index = rowOffset + column;

            float height = 0.25f * (
                tile.valueAt(index)
                + tile.valueAt(row + 1, column)
                + tile.valueAt(row, column + 1)
                + tile.valueAt(row + 1, column + 1));
            if (height < sandMaxHeight) {
                tile.setElement(index, sand);
                continue;
            }
            if (height < grasslandMaxHeight) {
                tile.setElement(index, grassland);
                continue;
            }
            tile.setElement(index, bedrock);
        }
    }
}
