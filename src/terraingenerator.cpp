#include "terraingenerator.h"

#include <random>
#include <cstdint>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <limits>
//
//#include <osg/Vec3f>
#include <osg/Shape>
#include <osgTerrain/Layer>
#include "osg/xzPlaneLocator.h"
#include <osgTerrain/TerrainTile>
#include <osgTerrain/Terrain>
//#include "osg/sharedgeometrytechnique.h"

#include <PxPhysics.h>
#include <PxRigidStatic.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>

#include "helper.h"

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
: sizeX(3.0f)
, sizeY(3.0f)
, columns(3)
, rows(3)
, tilesX(1u)
, tilesY(1u)
{
}

TerrainGenerator::TerrainGenerator()
: artifact(nullptr)
, m_heightSigma(0.2f)
, m_maxHeight(1.f)
{
}

ElemateHeightFieldTerrain * TerrainGenerator::generate() const
{
    ElemateHeightFieldTerrain * terrain = new ElemateHeightFieldTerrain(m_settings);
    
    if ((m_settings.tilesX != 1) || (m_settings.tilesY != 1))
        std::cerr << "Warning: creation of multiple terrain tiles not supported. Ignoring settings." << std::endl;

    osg::ref_ptr<osgTerrain::Terrain> osgTerrain = new osgTerrain::Terrain();
    terrain->m_osgTerrain = osgTerrain;

    //osgTerrain->setTerrainTechniquePrototype(new osgTerrain::SharedGeometryTechnique());

    {   // for each tile (...)
        osgTerrain::TileID tileID(0, 0, 0);

        osg::ref_ptr<osgTerrain::TerrainTile> tile = createTile(tileID);
        assert(tile.valid());
        osgTerrain->addChild(tile);

        osgTerrain::HeightFieldLayer * osgHeightField = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
        assert(osgHeightField);
        PxMat44 osgTransform(matrixOsgToPx(osgHeightField->getLocator()->getTransform()));
        PxMat44 osgTransformTranslated(matrixOsgToPxTranslate(osgHeightField->getLocator()->getTransform()));
        // transform physx height field coordinates to osg
        PxTransform rotate1(PxQuat(-std::_Pi / 2.0f, PxVec3(1, 0, 0)));
        PxTransform rotate2(PxQuat(-std::_Pi / 2.0f, PxVec3(0, 1, 0)));
        PxTransform mirror(PxMat44(PxVec4(-1, 1, -1, 1)));

        PxTransform transform(mirror * (rotate1 * rotate2));
        
        // center the terrain in the scene
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(transform);
        terrain->m_pxActors.emplace(tileID, actor);

        PxShape * pxShape = createPxShape(*osgHeightField->getHeightField(), *actor, PxTransform(), transform);
        terrain->m_pxShapes.emplace(tileID, pxShape);

        // debug the height geometries
        std::cout << "OSG x(right) y(front) z(up)" << std::endl;
        std::cout << std::setprecision(1);
        std::cout << std::fixed;
        for (unsigned c = 0; c < m_settings.columns; ++c) {
            std::cout << "Column " << c << std::endl;
            for (unsigned r = 0; r < m_settings.rows; ++r) {
                osg::Vec3 v = osgHeightField->getHeightField()->getVertex(c, r);
                osg::Vec3 vt = v * osgHeightField->getLocator()->getTransform();
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

void TerrainGenerator::setHeightSigma(float sigma)
{
    m_heightSigma = sigma;
}

float TerrainGenerator::heightSigma() const
{
    return m_heightSigma;
}

void TerrainGenerator::setMaxHeight(float height)
{
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

    //osg::ref_ptr<osgTerrain::xzPlaneLocator> locator = new osgTerrain::xzPlaneLocator();
    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();

    float xScaled = m_settings.sizeX * 0.5;
    float yScaled = m_settings.sizeY * 0.5;
    assert(m_settings.columns > 1);
    assert(m_settings.rows > 1);
    // locator->setTransformAsExtents() does only what the name suggests, when we set the intervalls as below
    // the "real" extents depend on the number of columns/rows
    //  ~ worldX = columns * (maxX-minX) + minX
    heightField->setXInterval(1.0f / (m_settings.columns - 1));
    heightField->setYInterval(1.0f / (m_settings.rows - 1));
    locator->setTransformAsExtents(-xScaled, -yScaled, xScaled, yScaled);

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
    float rowScale = 1.0 / (m_settings.rows - 1);
    float columnScale = 1.0 / (m_settings.columns -1);
    PxHeightFieldGeometry * m_pxHfGeometry = new  PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(), heightScaleToOsg, rowScale, columnScale);
    PxShape * shape = pxActor.createShape(*m_pxHfGeometry, mat, 1);


    // debug the height geometries
    std::cout << "phyx: x(right) y(up) z(back)" << std::endl;
    std::cout << std::setprecision(1);
    std::cout << std::fixed;

    for (unsigned c = 0; c < m_settings.columns; ++c) {
        std::cout << "Column " << c << std::endl;
        for (unsigned r = 0; r < m_settings.rows; ++r) {
            PxVec3 v = PxVec3(
                PxReal(r) * m_pxHfGeometry->rowScale, 
                PxReal(hfSamples[c + (r*m_settings.columns)].height) * m_pxHfGeometry->heightScale,
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

ElemateHeightFieldTerrain::ElemateHeightFieldTerrain(const TerrainSettings & settings)
: m_settings(settings)
{
}

osgTerrain::Terrain * ElemateHeightFieldTerrain::osgTerrain() const
{
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

