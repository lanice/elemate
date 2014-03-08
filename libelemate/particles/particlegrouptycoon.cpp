#include "particlegrouptycoon.h"

#include <cassert>
#include <list>

#include <glow/logging.h>
#include <glowutils/AxisAlignedBoundingBox.h>

#include "particlegroup.h"
#include "downgroup.h"
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

ParticleGroup * ParticleGroupTycoon::getNearestGroup(const std::string & elementName, const glm::vec3 & position)
{
    ParticleGroup * group = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto & pair : m_particleGroups)
    {
        if (!(pair.second->elementName() == elementName && pair.second->isDown))
            continue;
        float currentDistance = glm::distance(pair.second->boundingBox().center(), position);
        if (currentDistance >= nearestDistance)
            continue;
        nearestDistance = currentDistance;
        group = pair.second;
    }

    if (group != nullptr)
        return group;

    int id = ParticleScriptAccess::instance().createParticleGroup(false, elementName);

    return m_particleGroups.at(id);
}

void ParticleGroupTycoon::splitGroups()
{
    std::vector<glm::vec3> extractPositions;
    std::vector<glm::vec3> extractVelocities;
    std::vector<uint32_t> extractIndices;

    std::list<ParticleGroup*> newGroups;

    for (auto pair : m_particleGroups) {
        if (pair.second->numParticles() == 0 || dynamic_cast<DownGroup*>(pair.second) == nullptr)
            continue;

        const glowutils::AxisAlignedBoundingBox & bounds = pair.second->boundingBox();

        float splitValue;
        int splitAxis = longestAxis(bounds, splitValue);

        float longestLength = std::abs(bounds.urb()[splitAxis] - bounds.llf()[splitAxis]);
        
        assert(std::isfinite(longestLength));

        if (longestLength > 7.0f) {
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

            ParticleGroup * newGroup = new DownGroup(*pair.second);
            newGroup->createParticles(extractPositions, &extractVelocities);
            newGroups.push_back(newGroup);
        }
    }

    for (ParticleGroup * newGroup : newGroups) {
        ParticleScriptAccess::instance().addParticleGroup(newGroup);
    }

}
