#include "emittergroup.h"

#include <cassert>
#include <random>
#include <ctime>

#include <PxPhysics.h>
#include <PxScene.h>

#include <glowutils/AxisAlignedBoundingBox.h>

#include "particlegrouptycoon.h"
#include "rendering/particledrawable.h"
#include "terrain/terraininteraction.h"
#include "downgroup.h"
#include "io/soundmanager.h"


namespace {
    std::mt19937 rng;

    uint32_t seed_val = static_cast<uint32_t>(std::time(0));
    bool initRng() {
        rng.seed(seed_val);
        return true;
    }
    bool didRngInit = initRng();
}

using namespace physx;

EmitterGroup::EmitterGroup(const std::string & elementName, const unsigned int id, const bool enableGpuParticles, const uint32_t maxParticleCount,
    const ImmutableParticleProperties & immutableProperties, const MutableParticleProperties & mutableProperties)
: ParticleGroup(elementName, id, enableGpuParticles, false, maxParticleCount, immutableProperties, mutableProperties)
, m_emitting(false)
, m_timeSinceLastEmit(0.0)
{
    if (m_elementName == "lava")
        m_temperature = 700.0f;
    else if (m_elementName == "water")
        m_temperature = 10.0f;
    else
        m_temperature = 20.0f;
}

void EmitterGroup::emit(const float ratio, const glm::vec3 & position, const glm::vec3 & direction)
{
    m_emitRatio = ratio;
    m_emitPosition = position;
    m_emitDirection = glm::normalize(direction);
    m_emitting = true;
}

void EmitterGroup::stopEmit()
{
    m_emitting = false;
    m_timeSinceLastEmit = 0.0;
}

void EmitterGroup::updatePhysics(double delta)
{
    ParticleGroup::updatePhysics(delta);

    if (!m_emitting) return;

    unsigned int particlesToEmit = static_cast<unsigned int>(glm::floor(m_emitRatio * delta));

    std::uniform_real_distribution<float> uniform_dist(-0.75f, 0.75f);
    std::function<float()> scatterFactor = [&](){ return uniform_dist(rng); };

    if (particlesToEmit > 0)
    {
        for (unsigned int i = 0; i < particlesToEmit; ++i)
            createParticle(m_emitPosition, glm::vec3((m_emitDirection.x + scatterFactor()), (m_emitDirection.y + scatterFactor()), (m_emitDirection.z + scatterFactor())) * 100.f);

        m_timeSinceLastEmit = 0.0;
    }
    else {
        if (m_timeSinceLastEmit >= 1.0 / m_emitRatio)
        {
            createParticle(m_emitPosition, glm::vec3((m_emitDirection.x + scatterFactor()), (m_emitDirection.y + scatterFactor()), (m_emitDirection.z + scatterFactor())) * 100.f);
            m_timeSinceLastEmit = 0.0;
        }
        else {
            m_timeSinceLastEmit += delta;
        }
    }
}

void EmitterGroup::updateVisuals()
{
    ParticleGroup::updateVisuals();

    PxParticleReadData * readData = m_particleSystem->lockParticleReadData();
    assert(readData);

    m_particleDrawable->updateParticles(readData);

    m_particlesToDelete.clear();
    m_downPositions.clear();
    m_downVelocities.clear();

    // Get drained Particles
    PxStrideIterator<const PxParticleFlags> flagsIt(readData->flagsBuffer);
    PxStrideIterator<const PxVec3> positionIt = readData->positionBuffer;
    PxStrideIterator<const PxVec3> pxVelocityIt = readData->velocityBuffer;
    
    TerrainInteraction terrain("water");

    glowutils::AxisAlignedBoundingBox downBox;

    for (unsigned i = 0; i < readData->validParticleRange; ++i, ++flagsIt, ++positionIt, ++pxVelocityIt) {
        if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_STATIC) {
            const glm::vec3 & pos = reinterpret_cast<const glm::vec3&>(*positionIt.ptr());
            const glm::vec3 & vel = reinterpret_cast<const glm::vec3&>(*pxVelocityIt.ptr());
            m_downPositions.push_back(pos);
            m_downVelocities.push_back(vel);
            m_particlesToDelete.push_back(i);

            downBox.extend(pos);
        }
    }
    
    assert(m_numParticles == readData->nbValidParticles);
    readData->unlock();

    if (!m_particlesToDelete.empty()) {
        releaseParticles(m_particlesToDelete);

        DownGroup * group = ParticleGroupTycoon::instance().getNearestGroup(m_elementName, downBox.center());
        group->createParticles(m_downPositions, &m_downVelocities);
        group->setTemperature((m_numParticles * m_temperature + group->numParticles() * group->temperature()) / (m_numParticles + group->numParticles()));
    }
}
