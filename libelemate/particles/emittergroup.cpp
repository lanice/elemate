#include "emittergroup.h"

#include <cassert>

#include <PxPhysics.h>
#include <PxScene.h>

#include <glowutils/AxisAlignedBoundingBox.h>

#include "particlegrouptycoon.h"
#include "rendering/particledrawable.h"
#include "terrain/terraininteraction.h"

using namespace physx;

EmitterGroup::EmitterGroup(const std::string & elementName, const bool enableGpuParticles, const uint32_t maxParticleCount,
    const ImmutableParticleProperties & immutableProperties, const MutableParticleProperties & mutableProperties)
: ParticleGroup(elementName, enableGpuParticles, false, maxParticleCount, immutableProperties, mutableProperties)
{
    if (m_elementName == "lava")
        m_temperature = 700.0f;
    else if (m_elementName == "water")
        m_temperature = 10.0f;
    else
        m_temperature = 20.0f;
}

void EmitterGroup::updateVisuals()
{
    m_scene->removeActor(*m_particleSystem);

    m_particleSystem->setParticleReadDataFlag(PxParticleReadDataFlag::eVELOCITY_BUFFER, true);
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    m_particleSystem->setParticleReadDataFlag(PxParticleReadDataFlag::eVELOCITY_BUFFER, false);
    
    m_scene->addActor(*m_particleSystem);
    assert(readData);

    m_particlesToDelete.clear();
    m_downPositions.clear();
    m_downVelocities.clear();

    m_particleDrawable->updateParticles(readData);
        

    // Get drained Particles
    PxStrideIterator<const PxParticleFlags> flagsIt(readData->flagsBuffer);
    PxStrideIterator<const PxVec3> positionIt = readData->positionBuffer;
    PxStrideIterator<const PxVec3> pxVelocityIt = readData->velocityBuffer;
    
    TerrainInteraction terrain("water");

    glowutils::AxisAlignedBoundingBox downBox;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++flagsIt, ++positionIt, ++pxVelocityIt) {
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_STATIC) {
            if (positionIt->y < m_particleSize + 0.1)   // collision with water plane
            {
            }
            else if (!(terrain.topmostElementAt(positionIt->x, positionIt->z) == m_elementName))
            {
                glm::vec3 pos = glm::vec3(positionIt->x, positionIt->y, positionIt->z);
                m_downPositions.push_back(pos);
                glm::vec3 vel = glm::vec3(pxVelocityIt->x, pxVelocityIt->y, pxVelocityIt->z);
                m_downVelocities.push_back(vel);
                m_particlesToDelete.push_back(i);

                downBox.extend(pos);
            }
            else
            {
                m_particlesToDelete.push_back(i);
            }
        }
    }
    
    assert(m_numParticles == readData->nbValidParticles);
    readData->unlock();

    if (m_particlesToDelete.empty())
        return;
    releaseParticles(m_particlesToDelete);

    if (m_downPositions.empty())
        return;
    ParticleGroup * group = ParticleGroupTycoon::instance().getNearestGroup(m_elementName, downBox.center());
    group->createParticles(m_downPositions, &m_downVelocities);
}
