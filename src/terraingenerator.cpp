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
//#include <PxHeightFieldGeometry.h> 

#include <PxPhysics.h>
#include <cooking/PxCooking.h>
#include <extensions/PxDefaultStreams.h>
#include <geometry/PxTriangleMesh.h>
#include <geometry/PxTriangleMeshGeometry.h>

// Mersenne Twister, preconfigured
// keep one global instance, !per thread!

std::mt19937 rng;

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
{
}

osg::ref_ptr<osgTerrain::TerrainTile> TerrainGenerator::createTile(double xyScale, float heightScale, float heightSigma)
{
    osg::ref_ptr<osg::HeightField> heightField = new osg::HeightField;
    heightField->allocate(m_numColumns, m_numRows);

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

physx::PxTriangleMeshGeometry * TerrainGenerator::pxTerrainGeometry(const osgTerrain::Terrain * terrain)
{
    osg::ref_ptr<const osgTerrain::TerrainTile> tile = terrain->getTile(osgTerrain::TileID(0, 0, 0));
    osg::ref_ptr<const osgTerrain::SharedGeometryTechnique> technique =
        dynamic_cast<const osgTerrain::SharedGeometryTechnique*>(tile->getTerrainTechnique());
    if (technique.valid()) {
        osg::ref_ptr<osg::Geometry> geo = technique->getGeometry();
 /*       if (geo.valid())
            std::cout << "Terrain Primitive sets: " << geo->getNumPrimitiveSets() << std::endl;
        std::cout << "\tIndices: " << geo->getPrimitiveSet(0)->getNumIndices() << std::endl;*/

        osg::ref_ptr<const osg::Vec3Array> vertexData = dynamic_cast<osg::Vec3Array*>(geo->getVertexArray());
        if (!vertexData.valid()) {
            std::cerr << "Expected terrain vertexdata to be vec3array but wasn't" << std::endl;
            exit(2);
        }

        /*physx::PxDefaultMemoryInputData ogsData((physx::PxU8*) vertexData->getDataPointer(),
                            vertexData->size());*/

        physx::PxCookingParams cookingParams;
        // cookingParams...
        physx::PxCooking * cooking = PxCreateCooking(PX_PHYSICS_VERSION, PxGetPhysics().getFoundation(), cookingParams);
        physx::PxTriangleMeshDesc meshDesc;
        meshDesc.points = vertexData->getDataPointer();
        cooking->cookTriangleMesh(osgData, )
        physx::PxTriangleMesh * mesh = PxGetPhysics().createTriangleMesh(stream);
        physx::PxTriangleMeshGeometry * pGeo = new physx::PxTriangleMeshGeometry(mesh);
        return pGeo;
    }
    return nullptr;
}
