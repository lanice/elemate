#include "particlegrouptycoon.h"

#include <cassert>

#include <glow/logging.h>

#include "particlegroup.h"
#include "particlescriptaccess.h"
#include "particlehelper.h"

ParticleGroupTycoon * ParticleGroupTycoon::s_instance = nullptr;

void ParticleGroupTycoon::initialize()
{
    assert(s_instance == nullptr);
    s_instance = new ParticleGroupTycoon();
}

void ParticleGroupTycoon::release()
{
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

ParticleGroupTycoon & ParticleGroupTycoon::instance()
{
    assert(s_instance);
    return *s_instance;
}

ParticleGroupTycoon::ParticleGroupTycoon()
: m_timeSinceSplit(0.0)
{
    ParticleScriptAccess::initialize(m_particleGroups);
}

ParticleGroupTycoon::~ParticleGroupTycoon()
{
    for (auto pair : m_particleGroups)
        delete pair.second;

    m_particleGroups.clear();
}

void ParticleGroupTycoon::updatePhysics(double delta)
{
    for (auto pair : m_particleGroups)
        pair.second->updatePhysics(delta);

    m_timeSinceSplit += delta;
    if (m_timeSinceSplit > 1.0) {
        splitGroups();
        m_timeSinceSplit = 0.0;
    }
}

void ParticleGroupTycoon::updateVisuals()
{
    for (auto pair : m_particleGroups)
        pair.second->updateVisuals();
}

void ParticleGroupTycoon::splitGroups()
{
    std::vector<glm::vec3> extractPositions;
    std::vector<glm::vec3> extractVelocities;
    std::vector<uint32_t> extractIndices;

    for (auto pair : m_particleGroups) {
        if (pair.second->numParticles() == 0)
            continue;

        const glowutils::AxisAlignedBoundingBox & bounds = pair.second->boundingBox();

        float splitValue;
        int splitAxis = longestAxis(bounds, splitValue);

        float longestLength = std::abs(bounds.urb()[splitAxis] - bounds.llf()[splitAxis]);


        glow::debug("group size: %; with %; particles", longestLength, pair.second->numParticles());


        if (longestLength > 5.0f) {
            // extract the upper right back box
            glm::vec3 extractLlf = bounds.llf();
            extractLlf[splitAxis] = splitValue;
            glm::vec3 extractUrb = bounds.urb();

            glowutils::AxisAlignedBoundingBox extractBox(extractLlf, extractUrb);

            extractPositions.clear();
            extractIndices.clear();
            extractVelocities.clear();
            pair.second->particlePositionsIndicesVelocitiesInVolume(extractBox, extractPositions, extractIndices, extractVelocities);

            pair.second->releaseParticles(extractIndices);

            int newIndex = ParticleScriptAccess::instance().createParticleGroup(false, pair.second->elementName());
            m_particleGroups.at(newIndex)->createParticles(extractPositions, &extractVelocities);
        }
    }
}
