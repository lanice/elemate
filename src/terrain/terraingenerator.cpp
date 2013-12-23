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
        PxHeightFieldSample * pxBaseHeightFieldSamples = createBasicPxHeightField(0, m_settings.maxBasicHeightVariance);
        assert(pxBaseHeightFieldSamples);

        gougeRiverBed(pxBaseHeightFieldSamples);

        /** create terrain object and copy PhysX terrain data into it */
        BaseTile * baseTile = new BaseTile(*terrain, tileIDBase);
        copyPxHeightFieldToTile(*baseTile, pxBaseHeightFieldSamples);
        baseTile->createTerrainTypeTexture(pxBaseHeightFieldSamples);

        /** same thing for the water lever, just that we do not add a terrain type texture (it consists only of water) */
        TileID tileIDWater(TerrainLevel::WaterLevel, xID, zID);
        PxHeightFieldSample * pxWaterHeightFieldSamples = createBasicPxHeightField(1, 0);
        assert(pxWaterHeightFieldSamples);
        WaterTile * waterTile = new WaterTile(*terrain, tileIDWater);
        copyPxHeightFieldToTile(*waterTile, pxWaterHeightFieldSamples);

        /** Create physx objects: an actor with its transformed shapes
          * move tile according to its id, and by one half tile size, so the center of Tile(0,0,0) is in the origin */
        PxTransform pxTerrainTransform = PxTransform(PxVec3(m_settings.tileSizeX() * (xID - 0.5f), 0.0f, m_settings.tileSizeZ() * (zID - 0.5f)));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        terrain->m_pxActors.emplace(tileIDBase, actor);

        PxShape * pxBaseShape = createPxShape(*actor, pxBaseHeightFieldSamples);
        baseTile->m_pxShape = pxBaseShape;
        PxShape * pxWaterShape = createPxShape(*actor, pxWaterHeightFieldSamples);
        baseTile->m_pxShape = pxWaterShape;
    }

    return terrain;
}

PxHeightFieldSample * TerrainGenerator::createBasicPxHeightField(unsigned char defaultTerrainTypeId, float maxHeightVariance) const
{
    assert(m_settings.rows >= 2);
    assert(m_settings.columns >= 2);
    assert(m_settings.maxHeight > 0);
    assert(maxHeightVariance >= 0);
    assert(m_settings.maxHeight >= maxHeightVariance);

    float partHeight = maxHeightVariance / m_settings.maxHeight;
    std::uniform_int_distribution<> uniform_dist(
        static_cast<int>(std::numeric_limits<PxI16>::min() * partHeight),
        static_cast<int>(std::numeric_limits<PxI16>::max() * partHeight));

    std::function<int16_t()> getHeight;
    if (maxHeightVariance == 0)
        getHeight = [] () {return 0; };
    else
        getHeight = std::bind(uniform_dist, rng);

    unsigned int numSamples = m_settings.rows * m_settings.columns;

    // physx stores values in row major order (means starting with all values (per column) for the first row)
    PxHeightFieldSample* hfSamples = new PxHeightFieldSample[numSamples];
    for (unsigned i = 0; i < numSamples; ++i) {
        hfSamples[i].materialIndex0 = defaultTerrainTypeId;    // this clears also the tessellation flag
        hfSamples[i].materialIndex1 = defaultTerrainTypeId;
        hfSamples[i].height = getHeight();
    }

    return hfSamples;
}

void TerrainGenerator::gougeRiverBed(physx::PxHeightFieldSample * pxHfSamples) const
{
    std::function<float(float, float)> riverCourse = [](float normRow, float normColumn)
    {
        return abs(5 * powf(normRow, 3.0) - normColumn);
    };

    static const float riverScale = 0.15f;

    for (unsigned row = 0; row < m_settings.rows; ++row)
    {
        unsigned rowOffset = row * m_settings.columns;
        float normalizedRow = float(row) / m_settings.rows;
        for (unsigned column = 0; column < m_settings.columns; ++column)
        {
            float normalizedColumn = float(column) / m_settings.columns;
            unsigned index = column + rowOffset;
            PxI16 oldValue = pxHfSamples[index].height;
            float value = riverCourse(normalizedRow, normalizedColumn);
            if (value > riverScale)
                value = riverScale;
            value -= riverScale * 0.5f;
            pxHfSamples[index].height = static_cast<PxI16>(oldValue + std::numeric_limits<PxI16>::max() * (value - 0.5f*maxBasicHeightVariance()));
            int8_t terrainTypeID;
            if (pxHfSamples[index].height <= 0) {
                terrainTypeID = 2;  // this is dirt
            }
            else {
                terrainTypeID = 0; // this is something else.. bedrock or so
            }
            pxHfSamples[index].materialIndex0 = pxHfSamples[index].materialIndex1 = terrainTypeID;
        }
    }
}

PxShape * TerrainGenerator::createPxShape(PxRigidStatic & pxActor, const PxHeightFieldSample * hfSamples) const
{
    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbRows = m_settings.rows;
    hfDesc.nbColumns = m_settings.columns;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof( PxHeightFieldSample );

    PxHeightField * pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    PxMaterial * mat[4];
    // this is only for visuals testing: use some physx default material
    mat[0] = Elements::pxMaterial("default");
    mat[1] = Elements::pxMaterial("default");
    mat[2] = Elements::pxMaterial("default");
    mat[3] = Elements::pxMaterial("default");

    // scale height so that we use the full range of PxI16=short
    PxReal heightScale = m_settings.maxHeight / std::numeric_limits<PxI16>::max();
    assert(m_settings.intervalX() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    assert(m_settings.intervalZ() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    assert(heightScale >= PX_MIN_HEIGHTFIELD_Y_SCALE);
    // create height field geometry and set scale
    PxHeightFieldGeometry m_pxHfGeometry(pxHeightField, PxMeshGeometryFlags(),
        heightScale, m_settings.intervalX(), m_settings.intervalZ());
    PxShape * shape = pxActor.createShape(m_pxHfGeometry, mat, 1);

    assert(shape);

    return shape;
}

void TerrainGenerator::copyPxHeightFieldToTile(TerrainTile & tile, const PxHeightFieldSample * pxHeightFieldSamples) const
{
    unsigned int numSamples = m_settings.rows * m_settings.columns;

    const TileID & tileID = tile.m_tileID;

    /** create terrain data objects */

    glow::FloatArray * heightField = new glow::FloatArray();
    heightField->resize(numSamples);

    // compute position depending on TileID, which sets the row/column positions of the tile
    float minX = m_settings.tileSizeX() * (tileID.x - 0.5f);
    float minZ = m_settings.tileSizeZ() * (tileID.z - 0.5f);
    tile.m_transform = glm::mat4(
        m_settings.intervalX(), 0, 0, 0,
        0, 1, 0, 0,
        0, 0, m_settings.intervalZ(), 0,
        minX, 0, minZ, 1);

    float heightScale = m_settings.maxHeight / std::numeric_limits<PxI16>::max();

    for (unsigned index = 0; index < numSamples; ++index) {
        heightField->at(index) = pxHeightFieldSamples[index].height * heightScale;
    }

    tile.m_heightField = heightField;
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
