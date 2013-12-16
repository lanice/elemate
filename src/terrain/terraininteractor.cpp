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
    unsigned int osgColumn = physxRow;
    unsigned int osgRow = settings.columns - physxColumn - 1;

    osg::ref_ptr<osgTerrain::HeightFieldLayer> hfLayer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile.getElevationLayer());
    assert(hfLayer.valid());
    
    float value_inRange = value; /** clamp height value */
    if (value_inRange < -settings.maxHeight) value_inRange = -settings.maxHeight;
    if (value_inRange > settings.maxHeight) value_inRange = settings.maxHeight;

    hfLayer->getHeightField()->setHeight(osgColumn, osgRow, value_inRange);

    tile.setDirty(true);
    osgUtil::UpdateVisitor uv;
    uv.apply(tile);

    setPxHeight(tile.getTileID(), physxRow, physxColumn, value_inRange);

    return value_inRange;
}

void TerrainInteractor::setPxHeight(const osgTerrain::TileID & tileID, unsigned physxRow, unsigned physxColumn, float value)
{
    PxHeightFieldGeometry geometry;
    assert(m_terrain->pxShape(tileID)->getHeightFieldGeometry(geometry));
    PxHeightField * hf = geometry.heightField;

    PxHeightFieldSample samplesM[1];
    for (PxU32 i = 0; i < 1; i++)
    {
        samplesM[i].height = value / geometry.heightScale;
        samplesM[i].materialIndex0 = samplesM[i].materialIndex1 = 0;
    }

    PxHeightFieldDesc descM;
    descM.nbColumns = 1;
    descM.nbRows = 1;
    descM.samples.data = samplesM;
    descM.format = hf->getFormat();
    descM.samples.stride = hf->getSampleStride();
    descM.thickness = hf->getThickness();
    descM.convexEdgeThreshold = hf->getConvexEdgeThreshold();
    descM.flags = hf->getFlags();

    assert(hf->modifySamples(physxColumn, physxRow, descM)); // modify row 1 with new sample data

    PxHeightFieldGeometry newGeometry(hf, PxMeshGeometryFlags(), geometry.heightScale, geometry.rowScale, geometry.columnScale);
    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScene = static_cast<PxScene*>(malloc(sizeof(pxScene)));
    PxGetPhysics().getScenes(&pxScene, 1);
    pxScene->lockWrite();
    m_terrain->pxShape(tileID)->setGeometry(newGeometry);
    pxScene->unlockWrite();
}

std::shared_ptr<ElemateHeightFieldTerrain> TerrainInteractor::terrain() const
{
    return m_terrain;
}

void TerrainInteractor::setTerrain(std::shared_ptr<ElemateHeightFieldTerrain>& terrain)
{
    m_terrain = terrain;
}
