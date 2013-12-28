#include "terraininteractor.h"

#include <glow/logging.h>

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxShape.h>
#include <PxRigidStatic.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>

#include "terrain.h"
#include "terraintile.h"

using namespace physx;


TerrainInteractor::TerrainInteractor(std::shared_ptr<Terrain>& terrain)
: m_terrain(terrain)
{
}

float TerrainInteractor::heightAt(float worldX, float worldZ) const
{
    return m_terrain->heightAt(worldX, worldZ);
}

float TerrainInteractor::heightAt(float worldX, float worldZ, TerrainLevel level) const
{
    return m_terrain->heightAt(worldX, worldZ, level);
}

float TerrainInteractor::setHeight(float worldX, float worldZ, TerrainLevel level, float value)
{
    std::shared_ptr<TerrainTile> tile = nullptr;
    unsigned int physxRow;
    unsigned int physxColumn;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, physxRow, physxColumn))
        return 0.0f;

    assert(tile);

    return setHeight(*tile.get(), physxRow, physxColumn, value);
}

float TerrainInteractor::changeHeight(float worldX, float worldZ, TerrainLevel level, float delta)
{
    std::shared_ptr<TerrainTile> tile;
    unsigned int row;
    unsigned int column;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, row, column))
        return 0.0f;

    float height = tile->heightAt(row, column);

    return setHeight(*tile.get(), row, column, height + delta);
}

float TerrainInteractor::setHeight(TerrainTile & tile, unsigned row, unsigned column, float value)
{
    const TerrainSettings & settings = m_terrain->settings;

    // abort, if we are at the bounderies (because we set 3x3 values)
    if (row == 0 || column == 0 || row == settings.rows - 1 || column == settings.columns - 1)
        return 0.0f;
    
    float value_inRange = value; /** clamp height value */
    if (value_inRange < -settings.maxHeight) value_inRange = -settings.maxHeight;
    if (value_inRange > settings.maxHeight) value_inRange = settings.maxHeight;

    for (int u = -1; u <= 1; ++u) {
        for (int v = -1; v <= 1; ++v) {
            tile.setHeight(row + u, column + v, value_inRange);
        }
        tile.addBufferUpdateRange(column - 1 + (row + u)*settings.columns, 3);
    }

    setPxHeight(tile, row, column, value);

    return value_inRange;
}

void TerrainInteractor::setPxHeight(TerrainTile & tile, unsigned row, unsigned column, float value)
{
    PxShape & pxShape = *tile.pxShape();
    PxHeightFieldGeometry geometry;
    bool result = pxShape.getHeightFieldGeometry(geometry);
    assert(result);
    if (!result) {
        glow::warning("TerrainInteractor::setPxHeight could not get heightfield geometry from px shape");
        return;
    }
    PxHeightField * hf = geometry.heightField;

    PxHeightFieldSample samplesM[9];
    for (PxU32 i = 0; i < 9; i++)
    {
        samplesM[i].height = static_cast<PxI16>(value / geometry.heightScale);
        samplesM[i].materialIndex0 = samplesM[i].materialIndex1 = tile.pxMaterialIndexAt(row, column);
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

    bool success = hf->modifySamples(column - 1, row - 1, descM);
    assert(success);
    if (!success) {
        glow::warning("TerrainInteractor::setPxHeight could not modify heightfield.");
        return;
    }

    PxHeightFieldGeometry newGeometry(hf, PxMeshGeometryFlags(), geometry.heightScale, geometry.rowScale, geometry.columnScale);
    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScene = static_cast<PxScene*>(malloc(sizeof(pxScene)));
    PxGetPhysics().getScenes(&pxScene, 1);
    pxScene->lockWrite();
    pxShape.setGeometry(newGeometry);
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

std::shared_ptr<const Terrain> TerrainInteractor::terrain() const
{
    return m_terrain;
}

void TerrainInteractor::setTerrain(std::shared_ptr<Terrain>& terrain)
{
    m_terrain = terrain;
}
