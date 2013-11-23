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
#include <PxShape.h>
#include <geometry/PxHeightField.h>
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
: sizeX(10.0f)
, sizeZ(20.0f)
, rows(4)
, columns(3)
, tilesX(1u)
, tilesZ(1u)
, m_maxHeight(2.f)
{
}

TerrainGenerator::TerrainGenerator()
: artifact(nullptr)
, m_heightSigma(0.02f)
{
}

ElemateHeightFieldTerrain * TerrainGenerator::generate() const
{
    ElemateHeightFieldTerrain * terrain = new ElemateHeightFieldTerrain(m_settings);

    osg::ref_ptr<osgTerrain::Terrain> osgTerrain = new osgTerrain::Terrain();
    terrain->m_osgTerrain = osgTerrain.get();

    /** transforms osg's base vectors to physx/opengl logic
      * ! matrix is inverted, in osg logic
      * osg base in px: x is x, y is -z, z is y.
      * For z, we need 1 and not -1. This is because z in physx is back, y in osg is front, but we parse the
      * data samples both from 0 to max. So this axis gets inverted this way. */
    osg::Matrix osgBaseTransformToPx(
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1);

    terrain->m_osgTerrainTransform = new osg::MatrixTransform(osgBaseTransformToPx);
    terrain->m_osgTerrainTransform->addChild(osgTerrain.get());

    {   // for each tile (...)
        osgTerrain::TileID tileID(0, 0, 0);

        /** create terrain data and store it in PhysX terrain object */

        // center the terrain in the scene
        PxTransform pxTerrainTransform = PxTransform(PxVec3(-m_settings.sizeX / 2.0f, 0.0f, -m_settings.sizeZ / 2.0f));
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(pxTerrainTransform);
        terrain->m_pxActors.emplace(tileID, actor);

        PxHeightFieldSample * pxHeightFieldSamples = createPxHeightFieldData(m_settings.rows * m_settings.columns);
        assert(pxHeightFieldSamples);

        PxShape * pxShape = createPxShape(*actor, pxHeightFieldSamples, pxTerrainTransform);
        terrain->m_pxShapes.emplace(tileID, pxShape);


        /** create OSG terrain object and copy PhysX terrain data into it */

        osg::ref_ptr<osgTerrain::TerrainTile> tile = createTile(tileID, pxHeightFieldSamples);
        assert(tile.valid());
        osgTerrain->addChild(tile);

        osgTerrain::HeightFieldLayer * osgHeightField = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
        assert(osgHeightField);


        // debug the height geometries
        std::cout << "OSG x(right)columns y(front)row z(up)" << std::endl;
        std::cout << std::setprecision(1);
        std::cout << std::fixed;
        osg::Matrix transformOsgComlete = osgHeightField->getLocator()->getTransform() * osgBaseTransformToPx;
        // use osg way of columns and rows here
        for (unsigned c = 0; c < m_settings.rows; ++c) {
            std::cout << "Column " << c << std::endl;
            for (unsigned r = 0; r < m_settings.columns; ++r) {
                //osg::Vec3 o = osgHeightField->getHeightField()->getOrigin();
                osg::Vec3 v = osgHeightField->getHeightField()->getVertex(c, r);
                osg::Vec3 vt = osg::Vec3(c, r, 0) * transformOsgComlete;
                float height;
                osgHeightField->getInterpolatedValue(c / m_settings.rows, r / m_settings.columns, height);
                vt[2] = height;
                std::cout.width(5); std::cout << v.x() << " ";
                std::cout.width(5); std::cout << v.y() << " ";
                std::cout.width(5); std::cout << v.z() << "     ";

                std::cout.width(5); std::cout << vt.x() << " ";
                std::cout.width(5); std::cout << vt.y() << " ";
                std::cout.width(5); std::cout << vt.z() << std::endl;
            }
            std::cout << std::endl;
        }
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
    m_settings.rows = xSamplesui >= 2 ? xSamplesui : 2;
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
    m_settings.m_maxHeight = height;
}
float TerrainGenerator::maxHeight() const
{
    return m_settings.m_maxHeight;
}

PxHeightFieldSample * TerrainGenerator::createPxHeightFieldData(unsigned numSamples) const
{
    assert(m_settings.rows >= 2);
    assert(m_settings.columns >= 2);
    assert(m_settings.m_maxHeight > 0);

    //std::normal_distribution<float> normal_dist(0.0f, m_heightSigma);
    std::uniform_int_distribution<> uniform_dist(
        std::numeric_limits<PxI16>::min(),
        std::numeric_limits<PxI16>::max());

    // physx stores values in row major order (means starting with all values (per column) for the first row)
    PxHeightFieldSample* hfSamples = new PxHeightFieldSample[numSamples];
    for (unsigned i = 0; i < numSamples; ++i) {
        hfSamples[i].materialIndex0 = 0;
        hfSamples[i].materialIndex1 = 0;
        hfSamples[i].clearTessFlag();
        hfSamples[i].height = uniform_dist(rng);
    }
    return hfSamples;
}

PxShape * TerrainGenerator::createPxShape(PxRigidStatic & pxActor, const PxHeightFieldSample * hfSamples, const PxMat44 & transform) const
{
    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    // rows/columns inverse to match physx with osg/our definition, again
    hfDesc.nbRows = m_settings.rows;
    hfDesc.nbColumns = m_settings.columns;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof( PxHeightFieldSample ); // not better 0 ??

    PxHeightField * pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    PxMaterial * mat[1];
    mat[0] = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.1f);

    assert(m_settings.m_maxHeight > 0);
    // scale height so that we use the full range of PxI16=short
    PxReal heightScale = m_settings.m_maxHeight / std::numeric_limits<PxI16>::max();
    // create height field geometry and set scale
    PxHeightFieldGeometry * m_pxHfGeometry = new  PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(),
        heightScale, m_settings.intervalX(), m_settings.intervalZ());
    PxShape * shape = pxActor.createShape(*m_pxHfGeometry, mat, 1);

    // debug the height geometries
    std::cout << "phyx: x(right)rows y(up) z(back)columns" << std::endl;
    std::cout << std::setprecision(1);
    std::cout << std::fixed;

    for (unsigned r = 0; r < hfDesc.nbRows; ++r) {
        std::cout << "Row: " << r << std::endl;
        for (unsigned c = 0; c < hfDesc.nbColumns; ++c) {
            PxVec3 v = PxVec3(
                PxReal(r) * m_pxHfGeometry->rowScale,
                PxReal(hfSamples[c + ( r*hfDesc.nbColumns )].height) * m_pxHfGeometry->heightScale,
                PxReal(c) * m_pxHfGeometry->columnScale);
            PxVec3 vt = transform.transform(v);
            std::cout.width(5); std::cout << v.x << " ";
            std::cout.width(5); std::cout << v.y << " ";
            std::cout.width(5); std::cout << v.z << "     ";

            std::cout.width(5); std::cout << vt.x << " ";
            std::cout.width(5); std::cout << vt.y << " ";
            std::cout.width(5); std::cout << vt.z << std::endl;
        }
        std::cout << std::endl;
    }

    assert(shape);

    return shape;
}

osgTerrain::TerrainTile * TerrainGenerator::createTile(const osgTerrain::TileID & tileID, const PxHeightFieldSample * pxHfSamples) const
{
    osg::ref_ptr<osg::HeightField> heightField = new osg::HeightField;
    // invert columns <-> rows to match with physx
    heightField->allocate(m_settings.rows, m_settings.columns);

    float heightScale = m_settings.m_maxHeight / std::numeric_limits<PxI16>::max();

    // generate the hight field data
    for (unsigned c = 0; c < m_settings.columns; ++c)
    for (unsigned r = 0; r < m_settings.rows; ++r) {
        heightField->setHeight(r, c,
            PxReal(pxHfSamples[c + ( r*m_settings.columns )].height) * heightScale);
    }

    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();

    // compute extents depending on TileID, which sets the row/column positions of the tile

    // locator->setTransformAsExtents() does only what the name suggests, when we set the intervalls as below
    // the "real" extents depend on the number of columns/rows
    //  ~ worldX = columns * (maxX-minX) + minX
    // we need to use z as y for osg, otherwise the terrain generation will not work as it expects this orientation
    /*osg::Matrixd transform(
        m_settings.tileSizeX(), 0, 0, 0,
        0, m_settings.tileSizeZ(), 0, 0,
        0, 0, 1, 0,
        -m_settings.tileSizeX() * 0.5, -m_settings.tileSizeX() * 0.5, 0, 1);*/
    //locator->setTransform(transform);
    //locator->setTransformAsExtents(-xScaled, -zScaled, xScaled, zScaled);

    float xScaled = m_settings.sizeX * 0.5f;
    float zScaled = m_settings.sizeZ * 0.5f;

    heightField->setXInterval(m_settings.intervalX());
    heightField->setYInterval(m_settings.intervalZ());

    //locator->setTransform(osg::Matrixd());
    locator->setTransformAsExtents(-xScaled, -zScaled, xScaled, zScaled);

    osg::ref_ptr<osgTerrain::HeightFieldLayer> layer = new osgTerrain::HeightFieldLayer(heightField);

    layer->setLocator(locator.get());

    osgTerrain::TerrainTile * tile = new osgTerrain::TerrainTile();
    tile->setElevationLayer(layer.get());
    tile->setTileID(tileID);

    return tile;
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

