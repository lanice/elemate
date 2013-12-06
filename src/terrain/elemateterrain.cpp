#include "elemateterrain.h"

#include <cstdint>

#include <osg/MatrixTransform>
#include <osgTerrain/Terrain>

#include <PxShape.h>

using namespace physx;


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
, m_osgTerrain(nullptr)
, m_osgTerrainTransform(nullptr)
{
}

float ElemateHeightFieldTerrain::heightAt(float x, float z) const
{
    // currently for one tile only
    assert(m_settings.tilesX == 1 && m_settings.tilesZ == 1);
    float normalizedX = x / m_settings.sizeX + 0.5f;
    float normalizedY = 0.5f - z / m_settings.sizeZ;

    if (normalizedX < 0.0f || normalizedX > 1.0f
        || normalizedY < 0.0f || normalizedY > 1.0f)
        return 0.0f;

    osg::ref_ptr<osgTerrain::TerrainTile> tile = m_osgTerrain->getTile(osgTerrain::TileID(0, 0, 0));
    float height = 0;
    tile->getElevationLayer()->getInterpolatedValue(normalizedX, normalizedY, height);
    return height;
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

const std::map<osgTerrain::TileID, physx::PxRigidStatic*> ElemateHeightFieldTerrain::pxActorMap() const
{
    return m_pxActors;
}

const TerrainSettings & ElemateHeightFieldTerrain::settings() const
{
    return m_settings;
}
