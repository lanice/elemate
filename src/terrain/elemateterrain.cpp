#include "elemateterrain.h"

#include <cstdint>
#include <algorithm>

#include <osg/MatrixTransform>
#include <osgTerrain/Terrain>

#include <PxShape.h>

using namespace physx;

std::initializer_list<TerrainLevel> TerrainLevels = {
    TerrainLevel::BaseLevel,
    TerrainLevel::WaterLevel
};


TerrainSettings::TerrainSettings()
: sizeX(200)
, sizeZ(200)
, maxHeight(2.f)
, maxBasicHeightVariance(0.3f)
, rows(20)
, columns(20)
, tilesX(1)
, tilesZ(1)
{
}

ElemateHeightFieldTerrain::ElemateHeightFieldTerrain(const TerrainSettings & settings)
: m_settings(settings)
, m_osgTerrain(new osgTerrain::Terrain())
, m_osgTerrainBase(new osg::Group())
, m_osgTerrainWater(new osg::Group())
, m_osgTerrainTransform(new osg::MatrixTransform())
{
    /** transforms osg's base vectors to physx/opengl logic
    * ! matrix is inverted, in osg logic
    * osg base in px: x is x, y is -z, z is y. */
    m_osgTerrainTransform->setMatrix(osg::Matrix(
        1, 0, 0, 0,
        0, 0, -1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1));

    m_osgTerrainTransform->addChild(m_osgTerrain);
    m_osgTerrain->addChild(m_osgTerrainBase);
    m_osgTerrain->addChild(m_osgTerrainWater);
}

float ElemateHeightFieldTerrain::heightAt(float x, float z) const
{
    float normalizedX, normalizedZ;
    osgTerrain::TileID tileID;
    tileID.level = 0;
    if (!normalizePosition(x, z, tileID, normalizedX, normalizedZ))
        return 0.0f;

    float height = std::numeric_limits<float>::lowest();
    for (TerrainLevel level : TerrainLevels) {
        tileID.level = static_cast<int>(level);
        height = std::max(height, interpolatedHeightAt(tileID, normalizedX, normalizedZ));
    }

    return height;
}

float ElemateHeightFieldTerrain::heightAt(float x, float z, TerrainLevel level) const
{
    float normalizedX, normalizedZ;
    osgTerrain::TileID tileID;
    tileID.level = static_cast<int>(level);
    if (!normalizePosition(x, z, tileID, normalizedX, normalizedZ))
        return 0.0f;

    return interpolatedHeightAt(tileID, normalizedX, normalizedZ);
}

bool ElemateHeightFieldTerrain::normalizePosition(float x, float z, osgTerrain::TileID & tileID, float & normX, float & normZ) const
{
    // currently for one tile only
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);
    tileID.x = 0;
    tileID.y = 0;

    normX = x / m_settings.sizeX + 0.5f;
    normZ = z / m_settings.sizeZ + 0.5f;

    return normX >= 0.0f && normX <= 1.0f
        && normZ >= 0.0f && normZ <= 1.0f;
}

float ElemateHeightFieldTerrain::interpolatedHeightAt(osgTerrain::TileID tileID, float normalizedX, float normalizedZ) const
{
    // currently for one tile only
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);

    assert(normalizedX >= 0 && normalizedX <= 1);
    assert(normalizedZ >= 0 && normalizedZ <= 1);

    /** get position in osg logic: osgY axis is -physxZ axis */
    float osgNormalizedY = 1.0f - normalizedZ;

    float height;
    osg::ref_ptr<osgTerrain::TerrainTile> tile = m_osgTerrain->getTile(tileID);
    tile->getElevationLayer()->getInterpolatedValue(normalizedX, osgNormalizedY, height);

    return height;
}

float ElemateHeightFieldTerrain::heightAt(osgTerrain::TerrainTile & tile, unsigned int physxRow, unsigned int physxColumn) const
{
    osg::ref_ptr<osgTerrain::HeightFieldLayer> hfLayer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile.getElevationLayer());
    assert(hfLayer.valid());

    unsigned int osgColumn = physxRow;
    unsigned int osgRow = m_settings.columns - physxColumn - 1;

    return hfLayer->getHeightField()->getHeight(osgColumn, osgRow);
}

bool ElemateHeightFieldTerrain::worldToTileRowColumn(float x, float z, TerrainLevel level, osg::ref_ptr<osgTerrain::TerrainTile> & terrainTile, unsigned int & physxRow, unsigned int & physxColumn) const
{
    // only implemented for 1 tile
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);
    float normX = (x / m_settings.sizeX + 0.5f);
    float normZ = (z / m_settings.sizeZ + 0.5f);
    bool valid = normX >= 0 && normX <= 1 && normZ >= 0 && normZ <= 1;

    physxRow = static_cast<int>(normX * m_settings.rows) % m_settings.rows;
    physxColumn = static_cast<int>(normZ * m_settings.columns) % m_settings.columns;

    terrainTile = m_osgTerrain->getTile(osgTerrain::TileID(static_cast<int>(level), 0, 0));

    assert(terrainTile.valid());

    return valid;
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

osg::Group * ElemateHeightFieldTerrain::osgTerrainBase() const
{
    assert(m_osgTerrainBase.valid());
    return m_osgTerrainBase.get();
}

osg::Group * ElemateHeightFieldTerrain::osgTerrainWater() const
{
    assert(m_osgTerrainWater.valid());
    return m_osgTerrainWater.get();
}

PxShape const * ElemateHeightFieldTerrain::pxShape(const osgTerrain::TileID & tileID) const
{
    return m_pxShapes.at(tileID);
}

PxRigidStatic * ElemateHeightFieldTerrain::pxActor(const osgTerrain::TileID & tileID) const
{
    return m_pxActors.at(tileID);
}

const std::map<osgTerrain::TileID, physx::PxRigidStatic*> ElemateHeightFieldTerrain::pxActorMap() const
{
    return m_pxActors;
}

const TerrainSettings & ElemateHeightFieldTerrain::settings() const
{
    return m_settings;
}
