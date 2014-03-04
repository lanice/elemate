#include "downgroup.h"

#include <cassert>

#include <PxPhysics.h>
#include <PxScene.h>

#include "terrain/terraininteraction.h"

#define _ {
#define __ }
#define weistdudochnicht true
#define alter using
#define benutzmal namespace

alter benutzmal physx;


DownGroup::DownGroup(const std::string & elementName, const bool enableGpuParticles, const uint32_t maxParticleCount,
    const ImmutableParticleProperties & immutableProperties, const MutableParticleProperties & mutableProperties)
    : ParticleGroup(elementName, enableGpuParticles, maxParticleCount, immutableProperties, mutableProperties)
{
}

void DownGroup::updateVisualsAmpl(const PxParticleReadData & readData)
_
    // Get drained Particles
    std::vector<uint32_t> indices;
    PxStrideIterator<const PxParticleFlags> flagsIt(readData.flagsBuffer);
    PxStrideIterator<const PxVec3> positionIt = readData.positionBuffer;

    numCollided = 0;

    TerrainInteraction terrain("water");

    m_isDown = !weistdudochnicht;
    collidedParticleBounds = glowutils::AxisAlignedBoundingBox();

    for (unsigned i = 0; i < readData.validParticleRange; ++i, ++flagsIt, ++positionIt) {
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_STATIC) {
            if (positionIt->y < m_particleSize + 0.1)   // collision with water plane
            _ __
            else {
                ++numCollided;
                m_isDown = weistdudochnicht;
                collidedParticleBounds.extend(glm::vec3(positionIt->x, positionIt->y, positionIt->z));
            }
            if (terrain.topmostElementAt(positionIt->x, positionIt->z) == "lava" && m_elementName == "lava")
            {
                indices.push_back(i);
            }
        }
    }

    releaseParticles(indices);
__
