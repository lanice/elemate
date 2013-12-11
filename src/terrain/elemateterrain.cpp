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
    // currently for one tile only
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);
    float normalizedX = x / m_settings.sizeX + 0.5f;
    float normalizedZ = z / m_settings.sizeZ;

    if (normalizedX < 0.0f || normalizedX > 1.0f
        || normalizedZ < 0.0f || normalizedZ > 1.0f)
        return 0.0f;

    float height = std::numeric_limits<float>::lowest();
    for (TerrainLevel level : TerrainLevels) {
        height = std::max(height, heightAtNormalized(normalizedX, normalizedZ, TerrainLevel(level)));
    }

    return height;
}

float ElemateHeightFieldTerrain::heightAt(float x, float z, TerrainLevel level) const
{
    // currently for one tile only
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);
    float normalizedX = x / m_settings.sizeX + 0.5f;
    float normalizedZ = z / m_settings.sizeZ + 0.5f;

    if (normalizedX < 0.0f || normalizedX > 1.0f
        || normalizedZ < 0.0f || normalizedZ > 1.0f)
        return 0.0f;

    return heightAtNormalized(normalizedX, normalizedZ, level);
}

float ElemateHeightFieldTerrain::heightAtNormalized(float normalizedX, float normalizedZ, TerrainLevel level) const
{
    // currently for one tile only
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);

    assert(normalizedX >= 0 && normalizedX <= 1);
    assert(normalizedZ >= 0 && normalizedZ <= 1);

    /** get position in osg logic: osgY axis is -physxZ axis */
    float osgNormalizedY = 1.0f - normalizedZ;

    float height;
    osg::ref_ptr<osgTerrain::TerrainTile> tile = m_osgTerrain->getTile(osgTerrain::TileID(static_cast<int>(level), 0, 0));
    tile->getElevationLayer()->getInterpolatedValue(normalizedX, osgNormalizedY, height);

    return height;
}

bool ElemateHeightFieldTerrain::worldToTileRowColumn(float x, float z, osgTerrain::TileID & baseTileID, unsigned int & physxRow, unsigned int & physxColumn)
{
    // only implemented for 1 tile
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);
    float normX = (x / m_settings.sizeX + 0.5f);
    float normZ = (z / m_settings.sizeZ + 0.5f);
    bool valid = normX >= 0 && normX <= 1 && normZ >= 0 && normZ <= 1;

    physxRow = static_cast<int>(normX * m_settings.rows) % m_settings.rows;
    physxColumn = static_cast<int>(normZ * m_settings.columns) % m_settings.columns;

    baseTileID.level = static_cast<int>(TerrainLevel::BaseLevel);
    baseTileID.x = std::floorf(normX);
    baseTileID.y = std::floorf(normZ);

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
