#include "downgroup.h"

#include <cassert>

#include <PxPhysics.h>
#include <PxScene.h>

#include <glow/logging.h>
#include <glowutils/AxisAlignedBoundingBox.h>

#include "rendering/particledrawable.h"
#include "terrain/terraininteraction.h"
#include "terrain/terrain.h"
#include "particles/particlescriptaccess.h"
#include "particles/particlegrouptycoon.h"
#include "ui/achievementmanager.h"

#define alter using
#define benutzmal namespace
#define schwerkraftundso physx

alter benutzmal schwerkraftundso;


DownGroup::DownGroup(const std::string & elementName, const unsigned int id, const bool enableGpuParticles, const uint32_t maxParticleCount,
    const ImmutableParticleProperties & immutableProperties, const MutableParticleProperties & mutableProperties)
    : ParticleGroup(elementName, id, enableGpuParticles, true, maxParticleCount, immutableProperties, mutableProperties)
{ }


DownGroup::DownGroup(const ParticleGroup& lhs, unsigned int id)
: ParticleGroup(lhs, id)
{
}

DownGroup::DownGroup(const DownGroup& lhs, unsigned int id)
: ParticleGroup(lhs, id)
{
}

void DownGroup::updatePhysics(double delta)
{
ParticleGroup::updatePhysics(delta);
    if (m_elementName == "lava" && m_temperature < 690.f)
    {
        float temp = m_temperature;
        ParticleScriptAccess::instance().setUpParticleGroup(m_id, "bedrock");
        m_temperature = temp;
        m_elementName = "bedrock";
        m_particleDrawable->setElement(m_elementName);
        return;
    }

    if (m_elementName == "bedrock" && m_temperature > 710.f)
    {
        float temp = m_temperature;
        ParticleScriptAccess::instance().setUpParticleGroup(m_id, "lava");
        m_temperature = temp;
        m_elementName = "lava";
        m_particleDrawable->setElement(m_elementName);
        return;
    }

    if (m_elementName == "water" && m_temperature > 100.f)
    {
        float temp = m_temperature;
        ParticleScriptAccess::instance().setUpParticleGroup(m_id, "steam");
        m_temperature = temp;
        m_elementName = "steam";
        m_particleDrawable->setElement(m_elementName);

        AchievementManager::instance()->setProperty("steam", AchievementManager::instance()->getProperty("steam") + 1);
        return;
    }
}

void DownGroup::updateVisuals()
{
    ParticleGroup::updateVisuals();

    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    m_particleDrawable->updateParticles(readData);

    // Get drained Particles
    std::vector<uint32_t> particlesToDelete;
    PxStrideIterator<const PxParticleFlags> flagsIt(readData->flagsBuffer);
    PxStrideIterator<const PxVec3> positionIt = readData->positionBuffer;


    TerrainInteraction terrain("water");
    std::vector<unsigned int> lavaToSteam;
    glowutils::AxisAlignedBoundingBox steamBbox;
    std::vector<glm::vec3> steamPositions;

    const TerrainSettings & terrainSettings = terrain.terrain().settings;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++flagsIt, ++positionIt) {
        // check range
        if (positionIt->y > terrainSettings.maxHeight * 0.75f) {
            particlesToDelete.push_back(i);
            continue;
        }

        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_STATIC) {
            if (positionIt->y < m_particleSize + 0.1)   // collision with water plane
            {
                if (m_elementName == "lava")
                {
                    lavaToSteam.push_back(i);
                    steamBbox.extend(reinterpret_cast<const glm::vec3&>(*positionIt));
                    steamPositions.push_back(reinterpret_cast<const glm::vec3&>(*positionIt));
                    continue;
                } else {
                    particlesToDelete.push_back(i);
                    continue;
                }
            }
            if (terrain.topmostElementAt(positionIt->x, positionIt->z) == m_elementName)
            {
                particlesToDelete.push_back(i);
            }
        }
    }

    assert(m_numParticles == readData->nbValidParticles);
    readData->unlock();

    if (!particlesToDelete.empty())
        releaseParticles(particlesToDelete);

    if (!lavaToSteam.empty())
    {
        DownGroup * steamGroup = ParticleGroupTycoon::instance().getNearestGroup("steam", steamBbox.center());
        steamGroup->createParticles(steamPositions);
        releaseParticles(lavaToSteam);
    }
}
