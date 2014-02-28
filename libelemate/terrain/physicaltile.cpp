#include "physicaltile.h"

#include <glow/Program.h>
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

PhysicalTile::PhysicalTile(Terrain & terrain, const TileID & tileID, const std::initializer_list<std::string> & elementNames)
: TerrainTile(terrain, tileID, -terrain.settings.maxHeight, terrain.settings.maxHeight, 1.0f)
, m_elementNames(elementNames)
, m_pxShape(nullptr)
{
}

const std::string & PhysicalTile::elementAt(unsigned int row, unsigned int column) const
{
    return m_elementNames.at(elementIndexAt(row, column));
}


void PhysicalTile::bind(const CameraEx & camera)
{
    TerrainTile::bind(camera);

    if (!m_program)
        initializeProgram();

    assert(m_program);

    m_program->use();
    m_program->setUniform("cameraposition", camera.eye());
    glm::mat4 modelView = camera.view() * m_transform;
    m_program->setUniform("modelView", modelView);
    glm::mat4 modelViewProjection = camera.viewProjectionEx() * m_transform;
    m_program->setUniform("modelViewProjection", modelViewProjection);
    m_program->setUniform("znear", camera.zNearEx());
    m_program->setUniform("zfar", camera.zFarEx());
    m_terrain.setDrawGridOffsetUniform(*m_program, camera.eye());

    World::instance()->setUpLighting(*m_program);
}

void PhysicalTile::unbind()
{
    m_program->release();

    TerrainTile::unbind();
}

void PhysicalTile::initializeProgram()
{
    m_program->setUniform("modelTransform", m_transform);
    m_program->setUniform("heightField", 0);
    m_program->setUniform("tileSamplesPerAxis", int(samplesPerAxis));

    Elements::setAllUniforms(*m_program);
}

void PhysicalTile::setElement(unsigned int row, unsigned int column, const std::string & elementName)
{
    uint8_t index = elementIndex(elementName);
    setElement(row, column, index);
}

void PhysicalTile::updateBuffers()
{
    updatePxHeight();

    TerrainTile::updateBuffers();
}

using namespace physx;

PxShape * PhysicalTile::pxShape() const
{
    assert(m_pxShape);
    return m_pxShape;
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
            hfSamples[index].materialIndex0 = hfSamples[index].materialIndex1 = elementIndexAt(row, column);
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
            const float terrainHeight = valueAt(r + m_pxUpdateBox.minRow, c + m_pxUpdateBox.minColumn);
            samplesM[index].height = static_cast<PxI16>(terrainHeight / geometry.heightScale);
            samplesM[index].materialIndex0 = samplesM[index].materialIndex1 = elementIndexAt(r + m_pxUpdateBox.minRow, c + m_pxUpdateBox.minColumn);
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
