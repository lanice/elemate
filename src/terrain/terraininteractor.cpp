#include "terraininteractor.h"

#include <iostream>

#include <osgTerrain/Terrain>
#include <osgTerrain/Layer>
#include <osgUtil/UpdateVisitor>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxShape.h>
#include <PxRigidStatic.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>

#include "terrain/elemateterrain.h"
#include "osg/dynamicterraintile.h"

using namespace physx;


TerrainInteractor::TerrainInteractor(std::shared_ptr<ElemateHeightFieldTerrain>& terrain)
: m_terrain(terrain)
{
}

float TerrainInteractor::setHeight(float worldX, float worldZ, TerrainLevel level, float value)
{
    osg::ref_ptr<osgTerrain::TerrainTile> tile;
    unsigned int physxRow;
    unsigned int physxColumn;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, physxRow, physxColumn))
        return 0.0f;

    return setHeight(*tile.get(), physxRow, physxColumn, value);
}

float TerrainInteractor::changeHeight(float worldX, float worldZ, TerrainLevel level, float delta)
{
    osg::ref_ptr<osgTerrain::TerrainTile> tile;
    unsigned int physxRow;
    unsigned int physxColumn;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, physxRow, physxColumn))
        return 0.0f;

    float height = m_terrain->heightAt(*tile.get(), physxRow, physxColumn);

    return setHeight(*tile.get(), physxRow, physxColumn, height + delta);
}

float TerrainInteractor::setHeight(osgTerrain::TerrainTile & tile, unsigned physxRow, unsigned physxColumn, float value)
{
    const TerrainSettings & settings = m_terrain->settings();

    // abort, if we are at the bounderies (because we set 3x3 values)
    if (physxRow == 0 || physxColumn == 0 || physxRow == settings.rows - 1 || physxColumn == settings.columns - 1)
        return 0.0f;

    unsigned int osgColumn = physxRow;
    unsigned int osgRow = settings.columns - physxColumn - 1;
    
    float value_inRange = value; /** clamp height value */
    if (value_inRange < -settings.maxHeight) value_inRange = -settings.maxHeight;
    if (value_inRange > settings.maxHeight) value_inRange = settings.maxHeight;

    DynamicTerrainTile * dynamicTile = dynamic_cast<DynamicTerrainTile*>(&tile);
    assert(dynamicTile);

    for (int u = -1; u <= 1; ++u)
    for (int v = -1; v <= 1; ++v)
        dynamicTile->setHeightInGeometry(osgColumn + u, osgRow + v, value_inRange);

    dynamicTile->updateVBO();

    setPxHeight(tile.getTileID(), physxRow, physxColumn, value_inRange);

    return value_inRange;
}

void TerrainInteractor::setPxHeight(const osgTerrain::TileID & tileID, unsigned physxRow, unsigned physxColumn, float value)
{
    PxHeightFieldGeometry geometry;
    assert(m_terrain->pxShape(tileID)->getHeightFieldGeometry(geometry));
    PxHeightField * hf = geometry.heightField;

    PxHeightFieldSample samplesM[9];
    for (PxU32 i = 0; i < 9; i++)
    {
        samplesM[i].height = value / geometry.heightScale;
        samplesM[i].materialIndex0 = samplesM[i].materialIndex1 = 0;
    }

    PxHeightFieldDesc descM;
    descM.nbColumns = 3;
    descM.nbRows = 3;
    descM.samples.data = samplesM;
    descM.format = hf->getFormat();
    descM.samples.stride = hf->getSampleStride();
    descM.thickness = hf->getThickness();
    descM.convexEdgeThreshold = hf->getConvexEdgeThreshold();
    descM.flags = hf->getFlags();

    bool success = hf->modifySamples(physxColumn - 1, physxRow - 1, descM);
    assert(success); // modify row 1 with new sample data

    PxHeightFieldGeometry newGeometry(hf, PxMeshGeometryFlags(), geometry.heightScale, geometry.rowScale, geometry.columnScale);
    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScene = static_cast<PxScene*>(malloc(sizeof(pxScene)));
    PxGetPhysics().getScenes(&pxScene, 1);
    pxScene->lockWrite();
    m_terrain->pxShape(tileID)->setGeometry(newGeometry);
    pxScene->unlockWrite();
}

float TerrainInteractor::heightGrab(float worldX, float worldZ, TerrainLevel level)
{
    m_grabbedLevel = level;
    return m_grabbedHeight = m_terrain->heightAt(worldX, worldZ, level);
}

void TerrainInteractor::heightPull(float worldX, float worldZ)
{
    setHeight(worldX, worldZ, m_grabbedLevel, m_grabbedHeight);
}

std::shared_ptr<ElemateHeightFieldTerrain> TerrainInteractor::terrain() const
{
    return m_terrain;
}

void TerrainInteractor::setTerrain(std::shared_ptr<ElemateHeightFieldTerrain>& terrain)
{
    m_terrain = terrain;
}
