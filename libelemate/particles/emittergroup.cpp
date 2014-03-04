#include "emittergroup.h"

#include <cassert>

#include <PxPhysics.h>
#include <PxScene.h>

#include "terrain/terraininteraction.h"

using namespace physx;

EmitterGroup::EmitterGroup(const std::string & elementName, const bool enableGpuParticles, const uint32_t maxParticleCount,
    const ImmutableParticleProperties & immutableProperties, const MutableParticleProperties & mutableProperties)
: ParticleGroup(elementName, enableGpuParticles, maxParticleCount, immutableProperties, mutableProperties)
{
    if (m_elementName == "lava")
        m_temperature = 700.0f;
    else if (m_elementName == "water")
        m_temperature = 10.0f;
    else
        m_temperature = 20.0f;
}

void EmitterGroup::updateVisualsAmpl(const PxParticleReadData & readData)
{
    // Get drained Particles
    std::vector<uint32_t> indices;
    PxStrideIterator<const PxParticleFlags> flagsIt(readData.flagsBuffer);
    PxStrideIterator<const PxVec3> positionIt = readData.positionBuffer;

    numCollided = 0;

    TerrainInteraction terrain("water");

    m_isDown = false;
    collidedParticleBounds = glowutils::AxisAlignedBoundingBox();

    for (unsigned i = 0; i < readData.validParticleRange; ++i, ++flagsIt, ++positionIt) {
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_STATIC) {
            if (positionIt->y < m_particleSize + 0.1)   // collision with water plane
            {
            }
            else {
                ++numCollided;
                m_isDown = true;
                collidedParticleBounds.extend(glm::vec3(positionIt->x, positionIt->y, positionIt->z));
            }
            if (terrain.topmostElementAt(positionIt->x, positionIt->z) == "lava" && m_elementName == "lava")
            {
                indices.push_back(i);
            }
        }
    }

    releaseParticles(indices);
}
