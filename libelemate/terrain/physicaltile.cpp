#include "physicaltile.h"

#include <glow/Program.h>
#include <glow/Buffer.h>
#include <glow/Texture.h>
#include "utils/cameraex.h"

#include "utils/pxcompilerfix.h"
#include <PxPhysics.h>
#include <PxRigidStatic.h>
#include <PxShape.h>
#include <PxMaterial.h>
#include <PxScene.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldSample.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <foundation/PxMat44.h>
#ifdef PX_WINDOWS
#include <gpu/PxParticleGpu.h>
#endif

#include "utils/pxcompilerfix.h"
#include "physicswrapper.h"
#include "world.h"
#include "terrain.h"
#include "elements.h"
#include "texturemanager.h"

PhysicalTile::PhysicalTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames)
: TerrainTile(terrain, tileID, -terrain.settings.maxHeight, terrain.settings.maxHeight, 7, 1.0f)
, m_elementNames(elementNames)
, m_pxShape(nullptr)
{
    m_terrainTypeData.resize(samplesPerAxis * samplesPerAxis);
}

const std::string & PhysicalTile::elementAt(unsigned int row, unsigned int column) const
{
    assert(row < samplesPerAxis && column < samplesPerAxis);
    return m_elementNames.at(elementIndexAt(column + row * samplesPerAxis));
}

const std::string & PhysicalTile::elementAt(unsigned int tileValueIndex) const
{
    assert(tileValueIndex < samplesPerAxis * samplesPerAxis);
    return m_elementNames.at(elementIndexAt(tileValueIndex));
}

void PhysicalTile::initialize()
{
    TerrainTile::initialize();

    createTerrainTypeTexture();
}

void PhysicalTile::createTerrainTypeTexture()
{
    m_terrainTypeBuffer = new glow::Buffer(GL_TEXTURE_BUFFER);
    m_terrainTypeBuffer->setData(m_terrainTypeData, GL_DYNAMIC_DRAW);

    m_terrainTypeTex = new glow::Texture(GL_TEXTURE_BUFFER);
    m_terrainTypeTex->bind();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, m_terrainTypeBuffer->id());
    m_terrainTypeTex->unbind();

    m_terrainTypeTex->bindActive(GL_TEXTURE0 + TextureManager::reserveTextureUnit(tileName, "terrainType"));
    glActiveTexture(GL_TEXTURE0);
    CheckGLError();
}

void PhysicalTile::setElement(unsigned int row, unsigned int column, const std::string & elementName)
{
    uint8_t index = elementIndex(elementName);
    setElement(row, column, index);
}

void PhysicalTile::setElement(unsigned int row, unsigned int column, uint8_t elementIndex)
{
    setElement(column + row * samplesPerAxis, elementIndex);
}

void PhysicalTile::updateBuffers()
{
    assert(m_updateRangeMinMaxIndex.x < m_updateRangeMinMaxIndex.y);

    uint8_t * bufferDest = reinterpret_cast<uint8_t*>(m_terrainTypeBuffer->mapRange(
        m_updateRangeMinMaxIndex.x,
        m_updateRangeMinMaxIndex.y - m_updateRangeMinMaxIndex.x,
        GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    assert(bufferDest);

    unsigned int indexOffset = m_updateRangeMinMaxIndex.x;

    for (const UpdateRange & range : m_bufferUpdateList) {
        assert(indexOffset <= range.startIndex);
        assert(range.startIndex - indexOffset >= 0);
        assert(range.startIndex - indexOffset + range.nbElements < m_terrainTypeData.size());
        memcpy(bufferDest + (range.startIndex - indexOffset),
            reinterpret_cast<uint8_t*>(m_terrainTypeData.data()) + range.startIndex,
            range.nbElements);
    }

    m_terrainTypeBuffer->unmap();

    updatePxHeight();

    TerrainTile::updateBuffers();
}

using namespace physx;

PxShape * PhysicalTile::pxShape() const
{
    assert(m_pxShape);
    return m_pxShape;
}

uint8_t PhysicalTile::elementIndex(const std::string & elementName) const
{
    size_t index = std::find(m_elementNames.cbegin(), m_elementNames.cend(), elementName) - m_elementNames.cbegin();
    assert(index < m_elementNames.size());
    return static_cast<uint8_t>(index);
}

void PhysicalTile::createPxObjects(PxRigidStatic & pxActor)
{
    const unsigned int numSamples = samplesPerAxis * samplesPerAxis;

    // create the list of material references
    PxHeightFieldSample * hfSamples = new PxHeightFieldSample[numSamples];
    PxMaterial ** materials = new PxMaterial*[m_elementNames.size()];
    for (uint8_t i = 0; i < m_elementNames.size(); ++i)
        materials[i] = Elements::pxMaterial(m_elementNames.at(i));

    // scale height so that we use the full range of PxI16=short
    PxReal heightScaleToWorld = m_terrain.settings.maxHeight / std::numeric_limits<PxI16>::max();
    assert(heightScaleToWorld >= PX_MIN_HEIGHTFIELD_Y_SCALE);
    float heightScaleToPx = std::numeric_limits<PxI16>::max() / m_terrain.settings.maxHeight;

    // copy the material and height data into the physx height field
    for (unsigned int row = 0; row < samplesPerAxis; ++row) {
        const unsigned int rowOffset = row * samplesPerAxis;
        for (unsigned int column = 0; column < samplesPerAxis; ++column) {
            const unsigned int index = column + rowOffset;
            hfSamples[index].materialIndex0 = hfSamples[index].materialIndex1 = elementIndexAt(index);
            hfSamples[index].height = static_cast<PxI16>(m_values.at(index) * heightScaleToPx);
        }
    }

    PxHeightFieldDesc hfDesc;
    hfDesc.format = PxHeightFieldFormat::eS16_TM;
    hfDesc.nbRows = samplesPerAxis;
    hfDesc.nbColumns = samplesPerAxis;
    hfDesc.samples.data = hfSamples;
    hfDesc.samples.stride = sizeof(PxHeightFieldSample);

    PxHeightField * pxHeightField = PxGetPhysics().createHeightField(hfDesc);

    assert(sampleInterval >= PX_MIN_HEIGHTFIELD_XZ_SCALE);
    // create height field geometry and set scale
    PxHeightFieldGeometry pxHfGeometry(pxHeightField, PxMeshGeometryFlags(),
        heightScaleToWorld, sampleInterval, sampleInterval);
    m_pxShape = pxActor.createShape(pxHfGeometry, materials, 1);

    assert(m_pxShape);

#ifdef PX_WINDOWS
    if (PhysicsWrapper::physxGpuAvailable())
        PxParticleGpu::createHeightFieldMirror(*pxHeightField, *PhysicsWrapper::getInstance()->cudaContextManager());
#endif

    delete[] hfSamples;
    delete[] materials;
}

void PhysicalTile::clearBufferUpdateRange()
{
    m_pxUpdateBox = UIntBoundingBox();
}

PhysicalTile::UIntBoundingBox::UIntBoundingBox()
: minRow(std::numeric_limits<unsigned int>::max())
, maxRow(std::numeric_limits<unsigned int>::min())
, minColumn(std::numeric_limits<unsigned int>::max())
, maxColumn(std::numeric_limits<unsigned int>::min())
{
}

void PhysicalTile::addToPxUpdateBox(unsigned int minRow, unsigned int maxRow, unsigned int minColumn, unsigned int maxColumn)
{
    if (m_pxUpdateBox.minRow > minRow)
        m_pxUpdateBox.minRow = minRow;
    if (m_pxUpdateBox.maxRow < maxRow)
        m_pxUpdateBox.maxRow = maxRow;
    if (m_pxUpdateBox.minColumn > minColumn)
        m_pxUpdateBox.minColumn = minColumn;
    if (m_pxUpdateBox.maxColumn < maxColumn)
        m_pxUpdateBox.maxColumn = maxColumn;
}

void PhysicalTile::updatePxHeight()
{
    // do the physx update only when needed
    if (m_pxUpdateBox.maxColumn < m_pxUpdateBox.minColumn)
        return;

    PxHeightFieldGeometry geometry;
    bool result = m_pxShape->getHeightFieldGeometry(geometry);
    assert(result);
    if (!result) {
        glow::warning("TerrainInteractor::setPxHeight could not get height field geometry from physx shape");
        return;
    }
    PxHeightField * hf = geometry.heightField;

    assert(m_pxUpdateBox.minRow <= m_pxUpdateBox.maxRow && m_pxUpdateBox.minColumn <= m_pxUpdateBox.maxColumn);
    unsigned int nbRows = m_pxUpdateBox.maxRow - m_pxUpdateBox.minRow + 1;
    unsigned int nbColumns = m_pxUpdateBox.maxColumn - m_pxUpdateBox.minColumn + 1;
    unsigned int fieldSize = nbRows * nbColumns;

    PxHeightFieldSample * samplesM = new PxHeightFieldSample[fieldSize];
    for (unsigned int r = 0; r < nbRows; ++r) {
        unsigned int rowOffset = r * nbColumns;
        for (unsigned int c = 0; c < nbColumns; ++c) {
            const unsigned int index = c + rowOffset;
            const unsigned int tileValueIndex = (c + m_pxUpdateBox.minColumn) + (r + m_pxUpdateBox.minRow) * samplesPerAxis;
            const float terrainHeight = valueAt(tileValueIndex);
            samplesM[index].height = static_cast<PxI16>(terrainHeight / geometry.heightScale);
            samplesM[index].materialIndex0 = samplesM[index].materialIndex1 = elementIndexAt(tileValueIndex);
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

    bool success = hf->modifySamples(m_pxUpdateBox.minColumn, m_pxUpdateBox.minRow, descM);
    assert(success);
    if (!success) {
        glow::warning("TerrainInteractor::setPxHeight could not modify height field.");
        return;
    }

    PxHeightFieldGeometry newGeometry(hf, PxMeshGeometryFlags(), geometry.heightScale, geometry.rowScale, geometry.columnScale);
    assert(PxGetPhysics().getNbScenes() == 1);
    PxScene * pxScenePtrs[1];
    PxGetPhysics().getScenes(pxScenePtrs, 1);
    pxScenePtrs[0]->lockWrite();
    m_pxShape->setGeometry(newGeometry);
    pxScenePtrs[0]->unlockWrite();

    PhysicsWrapper::getInstance()->restoreGPUAccelerated();

#ifdef PX_WINDOWS
    if (PhysicsWrapper::getInstance()->physxGpuAvailable()) {
        PxParticleGpu::releaseHeightFieldMirror(*hf);
        PxParticleGpu::createHeightFieldMirror(*hf, *PhysicsWrapper::getInstance()->cudaContextManager());
    }
#endif
}
