#include "terraingenerator.h"

#include <random>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>

#include <osg/MatrixTransform>
#include <osgTerrain/Terrain>

#include <PxRigidStatic.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>

// Mersenne Twister, preconfigured
// keep one global instance, !per thread!

std::mt19937 rng;

using namespace physx;

namespace {
    uint32_t seed_val;
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

TerrainSettings::TerrainSettings()
: sizeX(20.0f)
, sizeZ(20.0f)
, columns(20)
, rows(20)
, tilesX(1u)
, tilesZ(1u)
{
}

TerrainGenerator::TerrainGenerator()
: artifact(nullptr)
, m_heightSigma(0.02f)
, m_maxHeight(5.f)
{
}

ElemateHeightFieldTerrain * TerrainGenerator::generate() const
{
    ElemateHeightFieldTerrain * terrain = new ElemateHeightFieldTerrain(m_settings);
    
    if ((m_settings.tilesX != 1) || (m_settings.tilesZ != 1))
        std::cerr << "Warning: creation of multiple terrain tiles not supported. Ignoring settings." << std::endl;

    osg::ref_ptr<osgTerrain::Terrain> osgTerrain = new osgTerrain::Terrain();
    terrain->m_osgTerrain = osgTerrain.get();

    // transforms osg's base vectors to physx/opengl logic
    // ! matrix is inverted, in osg logic
    osg::Matrix baseTransformOsgToPx(
        0, 0, 1, 0,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1);

    terrain->m_osgTerrainTransform = new osg::MatrixTransform(baseTransformOsgToPx);
    terrain->m_osgTerrainTransform->addChild(osgTerrain.get());

    {   // for each tile (...)
        osgTerrain::TileID tileID(0, 0, 0);

        osg::ref_ptr<osgTerrain::TerrainTile> tile = createTile(tileID);
        assert(tile.valid());
        osgTerrain->addChild(tile);

        osgTerrain::HeightFieldLayer * osgHeightField = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
        assert(osgHeightField);
        
        // center the terrain in the scene
        PxTransform pxTerrainTransform = PxTransform(PxVec3(-m_settings.sizeX / 2.0f, 0.0f, -m_settings.sizeZ / 2.0f));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        terrain->m_pxActors.emplace(tileID, actor);

        PxShape * pxShape = createPxShape(*osgHeightField->getHeightField(), *actor, PxTransform().createIdentity(), pxTerrainTransform);
        terrain->m_pxShapes.emplace(tileID, pxShape);
    }

    return terrain;
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

void TerrainGenerator::setSamplesPerWorldXCoord(float xSamples)
{
    assert(xSamples > 0.0f);
    unsigned int xSamplesui = unsigned int(ceil(m_settings.sizeX * xSamples));
    m_settings.columns = xSamplesui >= 2 ? xSamplesui : 2;
}

void TerrainGenerator::setSamplesPerWorldZCoord(float zSamples)
{
    assert(zSamples > 0.0f);
    unsigned int zSamplesui = unsigned int(ceil(m_settings.sizeZ * zSamples));
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
    assert(x >= 2 && z >= 2);
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

void TerrainGenerator::setHeightSigma(float sigma)
{
    assert(sigma >= 0.0f);
    m_heightSigma = sigma;
}

float TerrainGenerator::heightSigma() const
{
    return m_heightSigma;
}

void TerrainGenerator::setMaxHeight(float height)
{
    assert(height > 0.0f);
    m_maxHeight = height;
}
float TerrainGenerator::maxHeight() const
{
    return m_maxHeight;
}

osgTerrain::TerrainTile * TerrainGenerator::createTile(const osgTerrain::TileID & tileID) const
{
    osg::ref_ptr<osg::HeightField> heightField = new osg::HeightField;
    heightField->allocate(m_settings.columns, m_settings.rows);

    std::normal_distribution<float> normal_dist(0.0f, m_heightSigma);

    for (unsigned c = 0; c < m_settings.columns; ++c)
    for (unsigned r = 0; r < m_settings.rows; ++r) {
        float value = m_maxHeight * normal_dist(rng);
        assert(value <= m_maxHeight && value > -m_maxHeight);
        heightField->setHeight(c, r, value);
    }

    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();

    float xScaled = m_settings.sizeX * 0.5;
    float zScaled = m_settings.sizeZ * 0.5;
    assert(m_settings.columns > 1);
    assert(m_settings.rows > 1);
    // locator->setTransformAsExtents() does only what the name suggests, when we set the intervalls as below
    // the "real" extents depend on the number of columns/rows
    //  ~ worldX = columns * (maxX-minX) + minX
    heightField->setXInterval(1.0f / (m_settings.columns - 1));
    heightField->setYInterval(1.0f / (m_settings.rows - 1));
    locator->setTransformAsExtents(-xScaled, -zScaled, xScaled, zScaled);

    osg::ref_ptr<osgTerrain::HeightFieldLayer> layer = new osgTerrain::HeightFieldLayer(heightField);

    layer->setLocator(locator.get());

    osgTerrain::TerrainTile * tile = new osgTerrain::TerrainTile();
    tile->setElevationLayer(layer.get());
    tile->setTileID(tileID);

    return tile;
}

PxShape * TerrainGenerator::createPxShape(const osg::HeightField & osgHeightField, PxRigidStatic & pxActor, const PxTransform & scaling, const PxMat44 & transform) const
{
    assert(m_settings.rows > 0);
    assert(m_settings.columns > 0);
    assert(osgHeightField.getFloatArray());
    assert(osgHeightField.getFloatArray()->size() > 0);
    assert(m_maxHeight > 0);

    // osg uses float height, physx signed shorts, so we need to scale the values
    float heightScaleToPx = std::numeric_limits<PxI16>::max() / m_maxHeight;
    float heightScaleToOsg = m_maxHeight / std::numeric_limits<PxI16>::max();

    // physx stores values in row major order (means starting with all values (per column) for the first row)
    PxHeightFieldSample* hfSamples = new PxHeightFieldSample[m_settings.rows * m_settings.columns];
    for (unsigned c = 0; c < m_settings.columns; ++c)
    for (unsigned r = 0; r < m_settings.rows; ++r) {
        unsigned i = c + r * m_settings.columns;
        hfSamples[i].materialIndex0 = 0;
        hfSamples[i].materialIndex1 = 0;
        hfSamples[i].clearTessFlag();
        // scale float height
        hfSamples[i].height = osgHeightField.getHeight(c, r) * heightScaleToPx;
    }

    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbColumns = m_settings.columns;
    hfDesc.nbRows = m_settings.rows;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof(PxHeightFieldSample); // not better 0 ??

    PxHeightField * pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    PxMaterial * mat[1];
    mat[0] = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.1f);

    // create and scale height field geometry
    float rowScale = m_settings.intervalX();
    float columnScale = m_settings.intervalZ();
    PxHeightFieldGeometry * m_pxHfGeometry = new  PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(), heightScaleToOsg, rowScale, columnScale);
    PxShape * shape = pxActor.createShape(*m_pxHfGeometry, mat, 1);

    assert(shape);

    return shape;
}

ElemateHeightFieldTerrain::ElemateHeightFieldTerrain(const TerrainSettings & settings)
: m_settings(settings)
, m_osgTerrain(nullptr)
, m_osgTerrainTransform(nullptr)
{
}

osg::MatrixTransform * ElemateHeightFieldTerrain::osgTransformedTerrain() const
{
    assert(m_osgTerrainTransform.valid());
    assert(m_osgTerrainTransform->getChildIndex(m_osgTerrain) != m_osgTerrainTransform->getNumChildren());
    return m_osgTerrainTransform.get();
}

osgTerrain::Terrain * ElemateHeightFieldTerrain::osgTerrain() const
{
    assert(m_osgTerrain.valid());
    return m_osgTerrain.get();
}

PxShape const * ElemateHeightFieldTerrain::pxShape(const osgTerrain::TileID & tileID) const
{
    return m_pxShapes.at(tileID);
}

PxRigidStatic * ElemateHeightFieldTerrain::pxActor(const osgTerrain::TileID & tileID) const
{
    return m_pxActors.at(tileID);
}

