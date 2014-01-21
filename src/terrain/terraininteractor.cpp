#include "terraininteractor.h"

#include <algorithm>

#include <glow/logging.h>

#include "pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxScene.h>
#include <PxShape.h>
#include <PxRigidStatic.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#ifdef PX_WINDOWS
#include <gpu/PxParticleGpu.h>
#endif

#include "terrain.h"
#include "terraintile.h"
#include "physicswrapper.h"

using namespace physx;


TerrainInteractor::TerrainInteractor(std::shared_ptr<Terrain>& terrain, const std::string & interactMaterial)
: m_terrain(terrain)
, m_interactMaterial(interactMaterial)
, m_interactLevel(levelForMaterial(interactMaterial))
{
}

const std::string & TerrainInteractor::interactMaterial() const
{
    return m_interactMaterial;
}

void TerrainInteractor::setInteractMaterial(const std::string & material)
{
    m_interactMaterial = material;
    m_interactLevel = levelForMaterial(material);
}

float TerrainInteractor::heightAt(float worldX, float worldZ) const
{
    return m_terrain->heightAt(worldX, worldZ, m_interactLevel);
}

bool TerrainInteractor::isHeighestAt(float worldX, float worldZ) const
{
    if (TerrainLevels.size() == 1)
        return true;

    float othersMaxHeight = std::numeric_limits<float>::lowest();
    for (TerrainLevel level : TerrainLevels) {
        if (level == m_interactLevel)
            continue;
        othersMaxHeight = std::max(m_terrain->heightAt(worldX, worldZ, level), othersMaxHeight);
    }
    float myHeight = m_terrain->heightAt(worldX, worldZ, m_interactLevel);
    return othersMaxHeight < myHeight;
}

float TerrainInteractor::changeHeight(float worldX, float worldZ, float delta)
{
    return changeLevelHeight(worldX, worldZ, m_interactLevel, delta, 2);
}

float TerrainInteractor::terrainHeightAt(float worldX, float worldZ) const
{
    return m_terrain->heightTotalAt(worldX, worldZ);
}

float TerrainInteractor::levelHeightAt(float worldX, float worldZ, TerrainLevel level) const
{
    return m_terrain->heightAt(worldX, worldZ, level);
}

float TerrainInteractor::setLevelHeight(float worldX, float worldZ, TerrainLevel level, float value, unsigned int diameter)
{
    std::shared_ptr<TerrainTile> tile = nullptr;
    unsigned int physxRow;
    unsigned int physxColumn;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, physxRow, physxColumn))
        return 0.0f;

    assert(tile);

    return setHeight(*tile.get(), physxRow, physxColumn, value, diameter);
}

float TerrainInteractor::changeLevelHeight(float worldX, float worldZ, TerrainLevel level, float delta, unsigned int diameter)
{
    std::shared_ptr<TerrainTile> tile;
    unsigned int row;
    unsigned int column;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, row, column))
        return 0.0f;

    float height = tile->heightAt(row, column);

    return setHeight(*tile.get(), row, column, height + delta, diameter);
}

void TerrainInteractor::takeOffVolume(float worldX, float worldZ, float volume)
{
    const float heightDelta = - volume / (m_terrain->settings.intervalX() * m_terrain->settings.intervalZ());
    changeLevelHeight(worldX, worldZ, m_interactLevel, heightDelta, 2);
}

float TerrainInteractor::setHeight(TerrainTile & tile, unsigned row, unsigned column, float value, unsigned int diameter)
{
    assert(diameter > 0);
    const TerrainSettings & settings = m_terrain->settings;

    int maxD = diameter - int((diameter + 1) * 0.5f);
    int minD = maxD - diameter + 1;

    // abort, if we are at the bounderies
    if (row < static_cast<unsigned>(-minD) || column < static_cast<unsigned>(-minD) || row > settings.rows - maxD || column == settings.columns - maxD)
        return 0.0f;
    
    float value_inRange = value; /** clamp height value */
    if (value_inRange < -settings.maxHeight) value_inRange = -settings.maxHeight;
    if (value_inRange > settings.maxHeight) value_inRange = settings.maxHeight;

    for (int u = minD; u <= maxD; ++u) {
        for (int v = minD; v <= maxD; ++v) {
            tile.setHeight(row + u, column + v, value_inRange);
        }
        tile.addBufferUpdateRange(column - minD + (row + u)*settings.columns, diameter);
    }

    updatePxHeight(tile, row, column, diameter);

    return value_inRange;
}

void TerrainInteractor::updatePxHeight(TerrainTile & tile, unsigned row, unsigned column, unsigned int diameter)
{
    int maxD = diameter - int((diameter + 1) * 0.5f);
    int minD = maxD - diameter + 1;
    int sizeD = diameter * diameter;

    PxShape & pxShape = *tile.pxShape();
    PxHeightFieldGeometry geometry;
    bool result = pxShape.getHeightFieldGeometry(geometry);
    assert(result);
    if (!result) {
        glow::warning("TerrainInteractor::setPxHeight could not get heightfield geometry from px shape");
        return;
    }
    PxHeightField * hf = geometry.heightField;

    PxHeightFieldSample * samplesM = new PxHeightFieldSample[sizeD];
    for (unsigned int u = 0; u < diameter; ++u) {
        for (unsigned int v = 0; v < diameter; ++v) {
            const unsigned int index = u + v*diameter;
            const float terrainHeight = tile.heightAt(row + minD + u, column + minD + v);
            samplesM[index].height = static_cast<PxI16>(terrainHeight / geometry.heightScale);
            samplesM[index].materialIndex0 = samplesM[index].materialIndex1 = tile.pxMaterialIndexAt(row, column);
        }
    }

    PxHeightFieldDesc descM;
    descM.nbColumns = diameter;
    descM.nbRows = diameter;
    descM.samples.data = samplesM;
    descM.format = hf->getFormat();
    descM.samples.stride = hf->getSampleStride();
    descM.thickness = hf->getThickness();
    descM.convexEdgeThreshold = hf->getConvexEdgeThreshold();
    descM.flags = hf->getFlags();

    PhysicsWrapper::getInstance()->pauseGPUAcceleration();

    bool success = hf->modifySamples(column + minD, row + minD, descM);
    assert(success);
    if (!success) {
        glow::warning("TerrainInteractor::setPxHeight could not modify heightfield.");
        return;
    }

    PxHeightFieldGeometry newGeometry(hf, PxMeshGeometryFlags(), geometry.heightScale, geometry.rowScale, geometry.columnScale);
    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    pxScenePtrs[0]->lockWrite();
    pxShape.setGeometry(newGeometry);
    pxScenePtrs[0]->unlockWrite();

    PhysicsWrapper::getInstance()->restoreGPUAccelerated();

#ifdef PX_WINDOWS
    PxParticleGpu::releaseHeightFieldMirror(*hf);
    PxParticleGpu::createHeightFieldMirror(*hf, *PhysicsWrapper::getInstance()->cudaContextManager());
#endif
}

float TerrainInteractor::heightGrab(float worldX, float worldZ)
{
    m_grabbedLevel = m_interactLevel;
    return m_grabbedHeight = m_terrain->heightAt(worldX, worldZ, m_interactLevel);
}

void TerrainInteractor::heightPull(float worldX, float worldZ)
{
    setLevelHeight(worldX, worldZ, m_grabbedLevel, m_grabbedHeight, 3);
}

std::shared_ptr<const Terrain> TerrainInteractor::terrain() const
{
    return m_terrain;
}

void TerrainInteractor::setTerrain(std::shared_ptr<Terrain>& terrain)
{
    m_terrain = terrain;
}
