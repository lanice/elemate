#include "terraingenerator.h"

#include <random>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>
#include <ctime>
#include <functional>

#include <osg/MatrixTransform>
#include <osg/Image>
#include <osgTerrain/Terrain>

#include <PxRigidStatic.h>
#include <PxShape.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>

#include <elements.h>

// Mersenne Twister, preconfigured
// keep one global instance, !per thread!

std::mt19937 rng;

using namespace physx;

namespace {
    uint32_t seed_val = std::time(0);
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

TerrainGenerator::TerrainGenerator()
{
}

ElemateHeightFieldTerrain * TerrainGenerator::generate() const
{
    ElemateHeightFieldTerrain * terrain = new ElemateHeightFieldTerrain(m_settings);

    osg::ref_ptr<osgTerrain::Terrain> osgTerrain = new osgTerrain::Terrain();
    osgTerrain->setName("terrain root node");
    terrain->m_osgTerrain = osgTerrain.get();

    /** transforms osg's base vectors to physx/opengl logic
      * ! matrix is inverted, in osg logic
      * osg base in px: x is x, y is -z, z is y. */
    osg::Matrix terrainBaseOsgToPhysx(
        1, 0, 0, 0,
        0, 0, -1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1);

    terrain->m_osgTerrainTransform = new osg::MatrixTransform(terrainBaseOsgToPhysx);
    terrain->m_osgTerrainTransform->addChild(osgTerrain.get());


    /** The tileID determines the position of the current tile in the grid of tiles.
      * Tiles get shifted by -(numTilesPerAxis + 1)/2 so that we have the Tile(0,0,0) in the origin.
      */
    
    int maxxID = m_settings.tilesX - int((m_settings.tilesX + 1) * 0.5);
    int minxID = maxxID - m_settings.tilesX + 1;
    int maxzID = m_settings.tilesZ - int((m_settings.tilesZ + 1) * 0.5);
    int minzID = maxzID - m_settings.tilesZ + 1;

    terrain->minTileXID = minxID;
    terrain->minTileZID = minzID;

    for (int xID = minxID; xID <= maxxID; ++xID)
    for (int zID = minzID; zID <= maxzID; ++zID)
    {
        osgTerrain::TileID tileIDBase(TerrainLevel::BaseLevel, xID, zID);
        osgTerrain::TileID tileIDWater(TerrainLevel::WaterLevel, xID, zID);

        /** 1. Create physx terrain heightfield data.
            2. Add some landscape to it (change heightfield, add material information)
            3. Copy physx data to osg and create osg terrain tile.
            4. Create physx actor and shape.
            5. Set some uniforms, valid for the whole terrain. */
        
        PxHeightFieldSample * pxBaseHeightFieldSamples = createBasicPxHeightField(0, m_settings.maxBasicHeightVariance);
        PxHeightFieldSample * pxWaterHeightFieldSamples = createBasicPxHeightField(1, 0);
        assert(pxBaseHeightFieldSamples);
        assert(pxWaterHeightFieldSamples);

        gougeRiverBed(pxBaseHeightFieldSamples);

        /** create OSG terrain object and copy PhysX terrain data into it */
        osg::ref_ptr<osgTerrain::TerrainTile> baseTile = copyToOsgTile(tileIDBase, pxBaseHeightFieldSamples);
        osg::ref_ptr<osgTerrain::TerrainTile> waterTile = copyToOsgTile(tileIDWater, pxWaterHeightFieldSamples);
        assert(baseTile.valid());
        assert(waterTile.valid());
        baseTile->setTerrain(terrain->osgTerrain()); // tell the tile that it's part of our terrain object
        waterTile->setTerrain(terrain->osgTerrain()); // tell the tile that it's part of our terrain object
        waterTile->setBlendingPolicy(osgTerrain::TerrainTile::BlendingPolicy::ENABLE_BLENDING);
        osgTerrain->addChild(baseTile.get());
        osgTerrain->addChild(waterTile.get());

        /** move tile according to its id, and by one half tile size, so the center of Tile(0,0,0) is in the origin */
        PxTransform pxTerrainTransform = PxTransform(PxVec3(m_settings.tileSizeX() * (xID - 0.5), 0.0f, m_settings.tileSizeZ() * (zID - 0.5)));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        terrain->m_pxActors.emplace(tileIDBase, actor);

        PxShape * pxBaseShape = createPxShape(*actor, pxBaseHeightFieldSamples);
        PxShape * pxWaterShape = createPxShape(*actor, pxWaterHeightFieldSamples);
        terrain->m_pxShapes.emplace(tileIDBase, pxBaseShape);
        terrain->m_pxShapes.emplace(tileIDWater, pxWaterShape);
    }


    /** set some uniforms for the terrain */
    osg::ref_ptr<osg::StateSet> terrainStateSet = terrain->m_osgTerrain->getOrCreateStateSet();
    // texture unit 0 should be color layer 0 in all tiles
    osg::ref_ptr<osg::Uniform> terrainIDSampler = new osg::Uniform(osg::Uniform::Type::SAMPLER_2D, "terrainID");
    terrainIDSampler->set(0);
    terrainStateSet->addUniform(terrainIDSampler.get());
    terrainStateSet->addUniform(new osg::Uniform("tileSize", osg::Vec3(
        m_settings.tileSizeX(),
        m_settings.maxHeight,
        m_settings.tileSizeZ())));
    terrainStateSet->addUniform(new osg::Uniform("tileRowsColumns", osg::Vec2(m_settings.rows, m_settings.columns))); // ivec2
    // add all material shading matrices
    Elements::addAllUniforms(*terrainStateSet);

    return terrain;
}

PxHeightFieldSample * TerrainGenerator::createBasicPxHeightField(unsigned int defaultTerrainTypeId, float maxHeightVariance) const
{
    assert(m_settings.rows >= 2);
    assert(m_settings.columns >= 2);
    assert(m_settings.maxHeight > 0);
    assert(maxHeightVariance >= 0);
    assert(m_settings.maxHeight >= maxHeightVariance);

    float partHeight = maxHeightVariance / m_settings.maxHeight;
    std::uniform_int_distribution<> uniform_dist(
        std::numeric_limits<PxI16>::min() * partHeight,
        std::numeric_limits<PxI16>::max() * partHeight);

    std::function<int()> getHeight;
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
    //std::uniform_int_distribution<> rndRiverHoles(0, 4);

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
            value -= riverScale * 0.5;
            pxHfSamples[index].height = oldValue + std::numeric_limits<PxI16>::max() * (value - 0.5*maxBasicHeightVariance());

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

    // scale height so that we use the full range of PxI16=short (abs(min) = abs(max)+1)
    PxReal heightScale = m_settings.maxHeight / (-std::numeric_limits<PxI16>::min());
    assert(m_settings.intervalX() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    assert(m_settings.intervalZ() >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    assert(heightScale >= PX_MIN_HEIGHTFIELD_Y_SCALE);
    // create height field geometry and set scale
    PxHeightFieldGeometry * m_pxHfGeometry = new  PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(),
        heightScale, m_settings.intervalX(), m_settings.intervalZ());
    PxShape * shape = pxActor.createShape(*m_pxHfGeometry, mat, 1);

    assert(shape);

    return shape;
}

osgTerrain::TerrainTile * TerrainGenerator::copyToOsgTile(const osgTerrain::TileID & tileID, const PxHeightFieldSample * pxHfSamples) const
{
    /** create needed osg terrain objects */

    osg::ref_ptr<osg::HeightField> heightField = new osg::HeightField;
    // invert columns <-> rows to match with physx
    heightField->allocate(m_settings.rows, m_settings.columns);

    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();
    // compute position depending on TileID, which sets the row/column positions of the tile
    // using x/z here, which corresponds to the world coordinates used in physx
    float minX = m_settings.tileSizeX() * (tileID.x - 0.5);
    float minZ = m_settings.tileSizeZ() * (tileID.y - 0.5);
    locator->setTransform(osg::Matrixd(
        m_settings.tileSizeX(), 0, 0, 0,
        0, m_settings.tileSizeZ(), 0, 0,
        0, 0, 1, 0,
        minX, minZ, 0, 1));

    osg::ref_ptr<osgTerrain::HeightFieldLayer> layer = new osgTerrain::HeightFieldLayer(heightField);
    layer->setLocator(locator.get());

    osgTerrain::TerrainTile * tile = new osgTerrain::TerrainTile();
    tile->setElevationLayer(layer.get());
    tile->setTileID(tileID);
    tile->setRequiresNormals(true);

    float heightScale = m_settings.maxHeight / (-std::numeric_limits<PxI16>::min());

    /** physx uses 8 bit for material indices, the highest bit set the orientation of the diagonal.
    Use material0 only, so setting one terrain type per quad, containing two triangles. */
    // we have to use floats for the sampler, int won't work
    typedef float IDTexType;

    /** Create osg image for image layer, storing the terrain id for the graphics. */
    osg::ref_ptr<osg::Image> terrainTypeData = new osg::Image();
    terrainTypeData->allocateImage(m_settings.rows, m_settings.columns, 1, GL_RED, GL_FLOAT);
    assert(terrainTypeData->isDataContiguous());
    assert(terrainTypeData->getTotalDataSize() == m_settings.rows * m_settings.columns * sizeof(IDTexType));
    IDTexType * dataPtr = reinterpret_cast<IDTexType*>(terrainTypeData->data());

    const unsigned terrainTypeCount = 4;

    // osg column == physx row
    // osg row == numColumns - physx column - 1 (osg rows going to y, physx columns to z, where osgY == -physxZ)
    const unsigned numOsgRows = m_settings.columns;
    const unsigned numOsgColumns = m_settings.rows;
    for (unsigned physxRow = 0; physxRow < m_settings.rows; ++physxRow) {
        const unsigned physxRowOffset = physxRow * m_settings.columns;
        for (unsigned physxColumn = 0; physxColumn < m_settings.columns; ++physxColumn) {
            // set osg heightfield data, matching the physx rows/columns, both in row major order
            const unsigned osgColumn = physxRow;
            const unsigned osgRow = numOsgRows - 1 - physxColumn;

            /** copy height value from physx to osg */
            heightField->setHeight(osgColumn, osgRow,
                PxReal(pxHfSamples[physxColumn + physxRowOffset].height) * heightScale);

            /** same with terrain type id */
            unsigned int terrainTypeID = pxHfSamples[physxColumn + physxRowOffset].materialIndex0;
            // scale terrain id to 0..1 -> values greater than 1 seem not to work with osg::Image
            dataPtr[osgColumn + osgRow * numOsgColumns] = static_cast<IDTexType>(terrainTypeID) / (terrainTypeCount - 1);
        }
    }

    osg::ref_ptr<osgTerrain::ImageLayer> terrainTypeLayer = new osgTerrain::ImageLayer(terrainTypeData.get());
    tile->setColorLayer(0, terrainTypeLayer.get());

    assert(tile->getNumColorLayers() == 1);

    // set some tile specific uniforms
    osg::ref_ptr<osg::StateSet> tileStateSet = tile->getOrCreateStateSet();
    tileStateSet->addUniform(new osg::Uniform("tileID", tileID.x, tileID.y));   // ivec2
    tileStateSet->addUniform(new osg::Uniform("tileLeftFront", osg::Vec2f(minX, minZ))); // vec2

    return tile;
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
