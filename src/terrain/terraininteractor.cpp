#include "terraininteractor.h"

#include <algorithm>
#include <cmath>

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

float TerrainInteractor::normalDist(float x, float mean, float stddev)
{
    return  // n(x) = n(x) = 1 / (stddev*sqrt(2*pi)) * exp( - (x-mean)^2 / (2*stddev^2))
        1.0f / (stddev * std::sqrt(2.0f * glm::pi<float>()))
        * std::exp(-(x - mean) * (x - mean) / (2.0f * stddev * stddev));
}

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
    return changeLevelHeight(worldX, worldZ, m_interactLevel, delta);
}

float TerrainInteractor::terrainHeightAt(float worldX, float worldZ) const
{
    return m_terrain->heightTotalAt(worldX, worldZ);
}

float TerrainInteractor::levelHeightAt(float worldX, float worldZ, TerrainLevel level) const
{
    return m_terrain->heightAt(worldX, worldZ, level);
}

float TerrainInteractor::setLevelHeight(float worldX, float worldZ, TerrainLevel level, float value)
{
    std::shared_ptr<TerrainTile> tile = nullptr;
    unsigned int physxRow;
    unsigned int physxColumn;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, physxRow, physxColumn))
        return 0.0f;

    assert(tile);

    return setHeight(*tile.get(), physxRow, physxColumn, value);
}

float TerrainInteractor::changeLevelHeight(float worldX, float worldZ, TerrainLevel level, float delta)
{
    std::shared_ptr<TerrainTile> tile;
    unsigned int row;
    unsigned int column;

    if (!m_terrain->worldToTileRowColumn(worldX, worldZ, level, tile, row, column))
        return 0.0f;

    float height = tile->heightAt(row, column);

    return setHeight(*tile.get(), row, column, height + delta);
}

void TerrainInteractor::takeOffVolume(float worldX, float worldZ, float volume)
{
    const float heightDelta = -volume / (m_terrain->settings.sampleInterval() * m_terrain->settings.sampleInterval());
    changeLevelHeight(worldX, worldZ, m_interactLevel, heightDelta);
}

float TerrainInteractor::setHeight(TerrainTile & tile, unsigned row, unsigned column, float value)
{
    float stddev = 7.0f;
    assert(stddev > 0);

    const TerrainSettings & settings = m_terrain->settings;

    /** clamp height value */
    if (value < -settings.maxHeight) value = -settings.maxHeight;
    if (value > settings.maxHeight) value = settings.maxHeight;

    // define the size of the affected interaction area, in grid coords
    const float effectRadiusWorld = stddev * 3;
    const uint32_t effectRadius = static_cast<uint32_t>(std::ceil(effectRadiusWorld * settings.samplesPerWorldCoord())); // = 0 means to change only the value at (row,column)

    bool moveUp = (value - tile.heightAt(row, column)) > 0;
    int invert = moveUp ? 1 : -1;   // invert the curve if moving downwards

    float norm0 = normalDist(0, 0, stddev);
    float maxHeight = settings.maxHeight;
    std::function<float(float)> interactHeight = [stddev, norm0, value, maxHeight, invert] (float x) {
        return normalDist(x, 0, stddev)     // - normalize normDist to
                   / norm0                  //    normDist value at interaction center
               * (2 * maxHeight + 10)       // - scale to height range + 10 to omit norm values near 0
               * invert                     // - mirror the curve along the y axis if moving downwoard
               + value                      // - move along y so that value==0 => y==0
               - (2*maxHeight + 10) * invert;
    };

    unsigned int minRow, maxRow, minColumn, maxColumn;
    {
        // unchecked signed min/max values, possibly < 0 or > numRows/Column
        int iMinRow = row - effectRadius, iMaxRow = row + effectRadius, iMinColumn = column - effectRadius, iMaxColumn = column + effectRadius;
        // work on rows and column that are in range of the terrain tile settings and larger than 0
        minRow = iMinRow < 0 ? 0 : (iMinRow >= static_cast<signed>(settings.rows) ? settings.rows - 1 : static_cast<unsigned int>(iMinRow));
        maxRow = iMaxRow < 0 ? 0 : (iMaxRow >= static_cast<signed>(settings.rows) ? settings.rows - 1 : static_cast<unsigned int>(iMaxRow));
        minColumn = iMinColumn < 0 ? 0 : (iMinColumn >= static_cast<signed>(settings.columns) ? settings.columns - 1 : static_cast<unsigned int>(iMinColumn));
        maxColumn = iMaxColumn < 0 ? 0 : (iMaxColumn >= static_cast<signed>(settings.columns) ? settings.columns - 1 : static_cast<unsigned int>(iMaxColumn));
    }

    for (unsigned int r = minRow; r <= maxRow; ++r) {
        float relWorldX = (signed(r) - signed(row)) * settings.sampleInterval();
        for (unsigned int c = minColumn; c <= maxColumn; ++c) {
            float relWorldZ = (signed(c) - signed(column)) * settings.sampleInterval();

            float localRadius = std::sqrt(relWorldX*relWorldX + relWorldZ*relWorldZ);

            if (localRadius > effectRadiusWorld)  // interaction in a circle, not square
                continue;

            float newLocalHeight = interactHeight(localRadius);

            bool localMoveUp = newLocalHeight > tile.heightAt(r, c);
            // don't do anything if we pull up the terrain but the local heightpoint is already heigher than its calculated height. (vice versa)
            if (localMoveUp != moveUp)
                continue;

            tile.setHeight(r, c, newLocalHeight);
        }
        tile.addBufferUpdateRange(minColumn + r * settings.columns, 1u + effectRadius * 2u);
    }

    updatePxHeight(tile, minRow, maxRow, minColumn, maxColumn);

    return value;
}

void TerrainInteractor::updatePxHeight(const TerrainTile & tile, unsigned minRow, unsigned maxRow, unsigned minColumn, unsigned maxColumn)
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

    assert(minRow <= maxRow && minColumn <= maxColumn);
    unsigned int nbRows = maxRow - minRow + 1;
    unsigned int nbColumns = maxColumn - minColumn + 1;
    unsigned int fieldSize = nbRows * nbColumns;

    PxHeightFieldSample * samplesM = new PxHeightFieldSample[fieldSize];
    for (unsigned int r = 0; r < nbRows; ++r) {
        unsigned int rowOffset = r * nbColumns;
        for (unsigned int c = 0; c < nbColumns; ++c) {
            const unsigned int index = c + rowOffset;
            const float terrainHeight = tile.heightAt(r + minRow, c + minColumn);
            samplesM[index].height = static_cast<PxI16>(terrainHeight / geometry.heightScale);
            samplesM[index].materialIndex0 = samplesM[index].materialIndex1 = tile.pxMaterialIndexAt(r + minRow, c + minColumn);
        }
    }

    PxHeightFieldDesc descM;
    descM.nbColumns = nbColumns;
    descM.nbRows = nbRows;
    descM.samples.data = samplesM;
    descM.format = hf->getFormat();
    descM.samples.stride = hf->getSampleStride();
    descM.thickness = hf->getThickness();
    descM.convexEdgeThreshold = hf->getConvexEdgeThreshold();
    descM.flags = hf->getFlags();

    PhysicsWrapper::getInstance()->pauseGPUAcceleration();

    bool success = hf->modifySamples(minColumn, minRow, descM);
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
    if (PhysicsWrapper::getInstance()->physxGpuAvailable()) {
        PxParticleGpu::releaseHeightFieldMirror(*hf);
        PxParticleGpu::createHeightFieldMirror(*hf, *PhysicsWrapper::getInstance()->cudaContextManager());
    }
#endif
}

float TerrainInteractor::heightGrab(float worldX, float worldZ)
{
    m_grabbedLevel = m_interactLevel;
    return m_grabbedHeight = m_terrain->heightAt(worldX, worldZ, m_interactLevel);
}

void TerrainInteractor::heightPull(float worldX, float worldZ)
{
    setLevelHeight(worldX, worldZ, m_grabbedLevel, m_grabbedHeight);
}

std::shared_ptr<const Terrain> TerrainInteractor::terrain() const
{
    return m_terrain;
}

void TerrainInteractor::setTerrain(std::shared_ptr<Terrain>& terrain)
{
    m_terrain = terrain;
}
