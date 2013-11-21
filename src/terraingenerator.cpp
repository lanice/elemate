#include "terraingenerator.h"

#include <random>
#include <cstdint>
#include <iostream>
#include <cassert>
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
: sizeX(20.0f)
, sizeZ(20.0f)
, columns(100u)
, rows(100u)
, tilesX(1u)
, tilesZ(1u)
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
    
    if ((m_settings.tilesX != 1) || (m_settings.tilesZ != 1))
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
        PxMat44 transform = matrixOsgToPx(osgHeightField->getLocator()->getTransform());
        PxRigidStatic * actor = PxGetPhysics().createRigidStatic(PxTransform(transform));
        terrain->m_pxActors.emplace(tileID, actor);

        PxShape * pxShape = createPxShape(*osgHeightField->getHeightField(), *actor);
        terrain->m_pxShapes.emplace(tileID, pxShape);
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
        heightField->setHeight(c, r, m_maxHeight * normal_dist(rng));
    }

    //osg::ref_ptr<osgTerrain::xzPlaneLocator> locator = new osgTerrain::xzPlaneLocator();
    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();

    float xyMax = m_settings.sizeX / 2.0f;
    locator->setTransformAsExtents(-xyMax, -xyMax, xyMax, xyMax);

    osg::ref_ptr<osgTerrain::HeightFieldLayer> layer = new osgTerrain::HeightFieldLayer(heightField);

    layer->setLocator(locator.get());

    osgTerrain::TerrainTile * tile = new osgTerrain::TerrainTile();
    tile->setElevationLayer(layer.get());
    tile->setTileID(tileID);

    return tile;
}

PxShape * TerrainGenerator::createPxShape(const osg::HeightField & osgHeightField, PxRigidStatic & pxActor) const
{
    assert(m_settings.rows > 0);
    assert(m_settings.columns > 0);
    assert(osgHeightField.getFloatArray());
    assert(osgHeightField.getFloatArray()->size() > 0);

    PxHeightFieldSample* hfSamples = new PxHeightFieldSample[m_settings.rows * m_settings.columns];
    for (unsigned c = 0; c < m_settings.columns; ++c)
    for (unsigned r = 0; r < m_settings.rows; ++r) {
        unsigned i = c * r + r;
        hfSamples[i].materialIndex0 = 0;
        hfSamples[i].materialIndex1 = 0;
        hfSamples[i].clearTessFlag();
        hfSamples[i].height = osgHeightField.getHeight(c, r);
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

    PxHeightFieldGeometry * m_pxHfGeometry = new  PxHeightFieldGeometry(pxHeightField, PxMeshGeometryFlags(), 1.0f, 1.0f, 1.0f);
    PxShape * shape = pxActor.createShape(*m_pxHfGeometry, mat, 1);

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

