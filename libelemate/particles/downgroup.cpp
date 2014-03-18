#include "downgroup.h"

#include <cassert>

#include <PxPhysics.h>
#include <PxScene.h>

#include <glowutils/AxisAlignedBoundingBox.h>

#include "rendering/particledrawable.h"
#include "terrain/terraininteraction.h"
#include "particles/particlescriptaccess.h"
#include "particles/particlegrouptycoon.h"
#include "ui/achievementmanager.h"

#define _ {
#define __ }
#define alter using
#define benutzmal namespace
#define schwerkraftundso physx

alter benutzmal schwerkraftundso;


DownGroup::DownGroup(const std::string & elementName, const unsigned int id, const bool enableGpuParticles, const uint32_t maxParticleCount,
    const ImmutableParticleProperties & immutableProperties, const MutableParticleProperties & mutableProperties)
    : ParticleGroup(elementName, id, enableGpuParticles, true, maxParticleCount, immutableProperties, mutableProperties)
    _ __


DownGroup::DownGroup(const ParticleGroup& lhs, unsigned int id)
: ParticleGroup(lhs, id)
{
}

DownGroup::DownGroup(const DownGroup& lhs, unsigned int id)
: ParticleGroup(lhs, id)
{
}

void DownGroup::updatePhysics(double /*delta*/)
_
    if (m_elementName == "lava" && m_temperature < 690.f)
    _
        float temp = m_temperature;
        ParticleScriptAccess::instance().setUpParticleGroup(m_id, "bedrock");
        m_temperature = temp;
        m_elementName = "bedrock";
        m_particleDrawable->setElement(m_elementName);
        return;
    __
    
    if (m_elementName == "bedrock" && m_temperature > 710.f)
    _
        float temp = m_temperature;
        ParticleScriptAccess::instance().setUpParticleGroup(m_id, "lava");
        m_temperature = temp;
        m_elementName = "lava";
        m_particleDrawable->setElement(m_elementName);
        return;
    __

    if (m_elementName == "water" && m_temperature > 100.f)
    _
        float temp = m_temperature;
        ParticleScriptAccess::instance().setUpParticleGroup(m_id, "steam");
        m_temperature = temp;
        m_elementName = "steam";
        m_particleDrawable->setElement(m_elementName);

        AchievementManager::instance()->setProperty("steam", AchievementManager::instance()->getProperty("steam") + 1);
        return;
    __
__

void DownGroup::updateVisuals()
_
    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();

    m_particleDrawable->updateParticles(readData);
    
    // Get drained Particles
    std::vector<uint32_t> indices;
    PxStrideIterator<const PxParticleFlags> flagsIt(readData->flagsBuffer);
    PxStrideIterator<const PxVec3> positionIt = readData->positionBuffer;


    TerrainInteraction terrain("water");
    std::vector<unsigned int> lavaToSteam;
    glowutils::AxisAlignedBoundingBox steamBbox;
    std::vector<glm::vec3> steamPositions;


    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++flagsIt, ++positionIt) _
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_STATIC) _
            if (positionIt->y < m_particleSize + 0.1)   // collision with water plane
                _
                if (m_elementName == "lava")
                _
                    lavaToSteam.push_back(i);
                    steamBbox.extend(reinterpret_cast<const glm::vec3&>(*positionIt));
                    steamPositions.push_back(reinterpret_cast<const glm::vec3&>(*positionIt));
                __
            __
            else
            _ __
            if (terrain.topmostElementAt(positionIt->x, positionIt->z) ==  m_elementName)
            _
                indices.push_back(i);
            __ __ __

    assert(m_numParticles == readData->nbValidParticles);
    readData->unlock();

    if (!indices.empty())
        releaseParticles(indices);

    if (!lavaToSteam.empty())
    _
        DownGroup * steamGroup = ParticleGroupTycoon::instance().getNearestGroup("steam", steamBbox.center());
        steamGroup->createParticles(steamPositions);
        releaseParticles(lavaToSteam);
    __
__
