#include "particlegrouptycoon.h"

#include <cassert>
#include <list>

#include <glow/logging.h>
#include <glowutils/AxisAlignedBoundingBox.h>

#include "particlescriptaccess.h"
#include "particlecollision.h"
#include "particlegroup.h"
#include "downgroup.h"
#include "particlehelper.h"

ParticleGroupTycoon * ParticleGroupTycoon::s_instance = nullptr;

const float gridSize = 6.0f;

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
, m_collisions(nullptr)
, m_collisionCheckDelta(0.0)
{
    ParticleScriptAccess::initialize(m_particleGroups);
    m_collisions = std::make_shared<ParticleCollision>();
}

ParticleGroupTycoon::~ParticleGroupTycoon()
{
    for (auto pair : m_particleGroups)
        delete pair.second;

    ParticleScriptAccess::release();
    m_particleGroups.clear();
}

void ParticleGroupTycoon::updatePhysics(double delta)
{
    checkCollisions(delta);

    std::vector<unsigned int> groupsToDelete;
    groupsToDelete.reserve(10);
    for (auto pair : m_particleGroups) {
        ParticleGroup * group = pair.second;
        if (group->isDown && group->numParticles() == 0)
            groupsToDelete.push_back(pair.first);
        else
            group->updatePhysics(delta);
    }
    for (unsigned int index : groupsToDelete) {
        ParticleScriptAccess::instance().removeParticleGroup(index);
    }

    m_timeSinceSplit += delta;
    if (m_timeSinceSplit > 0.34) {

        splitGroups();
        mergeGroups();
        m_timeSinceSplit = 0.0;
    }
}

void ParticleGroupTycoon::updateVisuals()
{
    for (auto pair : m_particleGroups)
        pair.second->updateVisuals();
}

const std::unordered_map<unsigned int, ParticleGroup *> & ParticleGroupTycoon::particleGroupsById() const
{
    return m_particleGroups;
}

DownGroup * ParticleGroupTycoon::getNearestGroup(const std::string & elementName, const glm::vec3 & position)
{
    DownGroup * group = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto & pair : m_particleGroups)
    {
        if (!(pair.second->elementName() == elementName && pair.second->isDown))
            continue;
        float currentDistance = glm::distance(pair.second->boundingBox().center(), position);
        if (currentDistance >= nearestDistance)
            continue;
        nearestDistance = currentDistance;
        group = static_cast<DownGroup*>(pair.second);
    }

    if (group != nullptr)
        return group;

    int id = ParticleScriptAccess::instance().createParticleGroup(false, elementName);

    return static_cast<DownGroup*>(m_particleGroups.at(id));
}

ParticleGroup * ParticleGroupTycoon::particleGroupById(unsigned int id)
{
    auto it = m_particleGroups.find(id);
    assert(it != m_particleGroups.end());
    return it->second;
}

const ParticleGroup * ParticleGroupTycoon::particleGroupById(unsigned int id) const
{
    auto it = m_particleGroups.find(id);
    assert(it != m_particleGroups.end());
    return it->second;
}

void ParticleGroupTycoon::checkCollisions(double deltaTime)
{
    m_collisionCheckDelta += deltaTime;
    if (m_collisionCheckDelta > 0.5) {
        m_collisions->performCheck();
        m_collisionCheckDelta = 0.0;
    }
}

void ParticleGroupTycoon::splitGroups()
{
    std::vector<glm::vec3> extractPositions;
    std::vector<glm::vec3> extractVelocities;
    std::vector<uint32_t> extractIndices;

    std::list<ParticleGroup*> newGroups;

    for (auto pair : m_particleGroups) {
        if (pair.second->numParticles() == 0 || !pair.second->isDown)
            continue;

        const glowutils::AxisAlignedBoundingBox & bounds = pair.second->boundingBox();

        float splitValue;
        int splitAxis = longestAxis(bounds, splitValue);

        float longestLength = std::abs(bounds.urb()[splitAxis] - bounds.llf()[splitAxis]);
        
        assert(isfinite(longestLength));

        if (longestLength > gridSize) {
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

            const unsigned int id = ParticleScriptAccess::instance().m_id + newGroups.size();
            DownGroup * newGroup = new DownGroup(*pair.second, id);
            newGroup->createParticles(extractPositions, &extractVelocities);
            newGroups.push_back(newGroup);
        }
    }

    for (ParticleGroup * newGroup : newGroups) {
        ParticleScriptAccess::instance().addParticleGroup(newGroup);
    }

}

void ParticleGroupTycoon::mergeGroups()
{
    /*for (auto elementToMapPair : m_grid)
    {
        elementToMapPair.second.clear();
    }*/
    m_grid.clear();

    std::vector<unsigned int> groupsToRemove;
    groupsToRemove.reserve(10);
    for (auto pair : m_particleGroups) {
        if (pair.second->numParticles() == 0 || !pair.second->isDown)
            continue;

        const glowutils::AxisAlignedBoundingBox & bounds = pair.second->boundingBox();

        glm::vec3 center = bounds.center();

        uint64_t gridIndex = gridIndexFromPosition(center);

        DownGroup * gridGroup = particleGroupAtGridIndex(gridIndex, pair.second->elementName());

        if (gridGroup == nullptr)
        {
            insertGroupIntoGrid(static_cast<DownGroup*>(pair.second), gridIndex);
            continue;
        }

        if (gridGroup == pair.second)
            continue;

        pair.second->giveGiftTo(*gridGroup);

        groupsToRemove.push_back(pair.first);
    }

    for (unsigned int index : groupsToRemove)
        ParticleScriptAccess::instance().removeParticleGroup(index);
}

uint64_t ParticleGroupTycoon::gridIndexFromPosition(const glm::vec3 & position)
{
    uint64_t posX = static_cast<uint64_t>(position.x / gridSize);
    uint64_t posZ = static_cast<uint64_t>(position.z / gridSize);

    return (posX << 32) + posZ;
}

DownGroup * ParticleGroupTycoon::particleGroupAtGridIndex(uint64_t index, const std::string elementName)
{
    auto it = m_grid.find(elementName);
    if (it == m_grid.end())
        return nullptr;

    auto it2 = it->second.find(index);
    if (it2 == it->second.end())
        return nullptr;

    return it2->second;
}

void ParticleGroupTycoon::insertGroupIntoGrid(DownGroup * group, uint64_t index)
{
    const std::string & elementName = group->elementName();
    auto it = m_grid.emplace(elementName, std::unordered_map<uint64_t, DownGroup*>());

    auto it2 = it.first->second.emplace(index, group);
    assert(it2.second);
}
