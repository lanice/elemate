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

osg::ref_ptr<osgTerrain::Terrain> TerrainGenerator::getTerrain()
{
    osg::HeightField * heightField = new osg::HeightField;
    heightField->allocate(m_numColumns, m_numRows);

    std::normal_distribution<float> normal_dist(0.0f, 0.2f);

    for (int c = 0; c < m_numColumns; ++c)
    for (int r = 0; r < m_numRows; ++r) {
        heightField->setHeight(c, r, normal_dist(rng));
    }

    osgTerrain::Locator * locator = new osgTerrain::Locator();
    //locator->setCoordinateSystemType(osgTerrain::Locator::GEOCENTRIC);
    double scale = 10.0;
    locator->setTransformAsExtents(-scale, -scale, scale, scale);

    osgTerrain::HeightFieldLayer * layer = new osgTerrain::HeightFieldLayer(heightField);

    layer->setLocator(locator);

    osgTerrain::TerrainTile * tile = new osgTerrain::TerrainTile();
    tile->setElevationLayer(layer);

    osgTerrain::Terrain * terrain = new osgTerrain::Terrain();

    terrain->addChild(tile);
    //terrain->updateTerrainTileOnNextFrame(tile);

    return terrain;
}
