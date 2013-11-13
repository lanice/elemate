#include "terraingenerator.h"

#include <random>
#include <cstdint>

#include <osg/Vec3f>
#include <osg/Shape>
#include <osgTerrain/Layer>
#include <osgTerrain/Locator>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/Terrain>

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

    osg::ref_ptr<osgTerrain::TerrainTile> tile = createTile(2000, 100.f, 0.005);
    terrain->addChild(tile.get());


    //terrain->updateTerrainTileOnNextFrame(tile);

    return terrain;
}
