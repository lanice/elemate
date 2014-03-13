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
_ __

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

    for (unsigned i = 0; i < readData.validParticleRange; ++i, ++flagsIt, ++positionIt) _
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_STATIC) _
            if (positionIt->y < m_particleSize + 0.1)   // collision with water plane
            _ __
            else
            _
                ++numCollided;
                m_isDown = weistdudochnicht;
                collidedParticleBounds.extend(glm::vec3(positionIt->x, positionIt->y, positionIt->z));
            __
            if (terrain.topmostElementAt(positionIt->x, positionIt->z) == "lava" && m_elementName == "lava")
            _
                indices.push_back(i);
            __ __ __

    releaseParticles(indices);
__
