#include "terraingenerator.h"

#include <random>
#include <cstdint>
#include <iostream>

#include <osg/Vec3f>
#include <osg/Shape>
#include <osgTerrain/Layer>
#include <osgTerrain/Locator>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/Terrain>
#include "sharedgeometrytechnique.h"

#include <PxPhysics.h>
//#include <cooking/PxCooking.h>
#include <foundation/PxTransform.h>
//#include <extensions/PxDefaultStreams.h>
#include <extensions/PxSimpleFactory.h>
//#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <PxMaterial.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
//#include <geometry/PxTriangleMesh.h>
//#include <geometry/PxTriangleMeshGeometry.h>

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

TerrainGenerator::TerrainGenerator(int numColumns, int numRows)
: m_numColumns(numColumns)
, m_numRows(numRows)
, m_elemateTerrain(new ElemateHeightFieldTerrain())
{
}

osg::ref_ptr<osgTerrain::TerrainTile> TerrainGenerator::createTile(double xyScale, float heightScale, float heightSigma)
{
    osg::ref_ptr<osg::HeightField> heightField = new osg::HeightField;
    heightField->allocate(m_numColumns, m_numRows);



    m_elemateTerrain->m_osgHeightField = heightField;



    std::normal_distribution<float> normal_dist(0.0f, heightSigma);

    for (int c = 0; c < m_numColumns; ++c)
    for (int r = 0; r < m_numRows; ++r) {
        heightField->setHeight(c, r, heightScale * normal_dist(rng));
    }

    osg::ref_ptr<osgTerrain::Locator> locator = new osgTerrain::Locator();

    double scale = xyScale * 0.5;
    locator->setTransformAsExtents(-scale, -scale, scale, scale);

    osg::ref_ptr<osgTerrain::HeightFieldLayer> layer = new osgTerrain::HeightFieldLayer(heightField);

    layer->setLocator(locator.get());

    osg::ref_ptr<osgTerrain::TerrainTile> tile = new osgTerrain::TerrainTile();
    tile->setElevationLayer(layer.get());

    return tile;
}

osg::ref_ptr<osgTerrain::Terrain> TerrainGenerator::getTerrain()
{
    osg::ref_ptr<osgTerrain::Terrain> terrain = new osgTerrain::Terrain();
    terrain->setTerrainTechniquePrototype(new osgTerrain::SharedGeometryTechnique());

    osg::ref_ptr<osgTerrain::TerrainTile> tile = createTile(2000, 100.f, 0.005);
    tile->setTileID(osgTerrain::TileID(0, 0, 0));
    //tile->setTerrain(terrain.get());
    terrain->addChild(tile.get());

    //tile->setTerrainTechnique(new osgTerrain::SharedGeometryTechnique());
    //tile->init(0, false);

    //terrain->updateTerrainTileOnNextFrame(tile);

    return terrain;
}

ElemateHeightFieldTerrain * TerrainGenerator::createHeightFieldTerrain()
{
    m_elemateTerrain->m_osgTerrain = getTerrain();
    m_elemateTerrain->m_numRows = m_numRows;
    m_elemateTerrain->m_numColumns = m_numColumns;
    m_elemateTerrain->m_rowScale = m_elemateTerrain->m_colScale = 1.0f;
    m_elemateTerrain->m_heightScale = 1.0f;
    m_elemateTerrain->heightFieldShape(); // creates it...
    return m_elemateTerrain;
}

ElemateHeightFieldTerrain::ElemateHeightFieldTerrain()
: m_actor(PxGetPhysics().createRigidStatic(PxTransform()))
, m_shape(nullptr)
, m_pxHeightField(nullptr)
, m_pxHfGeometry(nullptr)
{
}

PxRigidStatic * ElemateHeightFieldTerrain::actor() const
{
    return m_actor;
}

PxShape * ElemateHeightFieldTerrain::heightFieldShape() {
    if (m_shape != nullptr)
        return m_shape;

    PxHeightFieldSample* hfSamples = new PxHeightFieldSample[m_numRows*m_numColumns];
    for (unsigned c = 0; c < m_numColumns; ++c) 
    for (unsigned r = 0; r < m_numRows; ++r) {
        unsigned i = c * r + r;
        hfSamples[i].materialIndex0 = 0;
        hfSamples[i].materialIndex1 = 0;
        hfSamples[i].clearTessFlag();
        hfSamples[i].height = m_osgHeightField->getHeight(c, r);
    }

    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbColumns = m_numColumns;
    hfDesc.nbRows = m_numRows;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof( PxHeightFieldSample ); // not better 0 ??

    m_pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    PxMaterial * mat[1];
    mat[0] = PxGetPhysics().createMaterial(0.5f, 0.5f, 0.1f);

    m_pxHfGeometry = new  PxHeightFieldGeometry(m_pxHeightField, PxMeshGeometryFlags(), m_heightScale, m_rowScale, m_colScale);
    m_shape = actor()->createShape(*m_pxHfGeometry, mat, 1);

    return m_shape;
}
