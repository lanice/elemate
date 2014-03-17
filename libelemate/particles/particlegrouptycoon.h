#pragma once

#include <unordered_map>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

class ParticleGroup;
class DownGroup;
class ParticleCollision;

class ParticleGroupTycoon
{
public:
    static void initialize();
    static void release();

    static ParticleGroupTycoon & instance();

    void updatePhysics(double delta);
    void updateVisuals();

    ParticleGroup * getNearestGroup(const std::string & elementName, const glm::vec3 & position);
    ParticleGroup * particleGroupById(unsigned int id);
    const ParticleGroup * particleGroupById(unsigned int id) const;

    const std::unordered_map<unsigned int, ParticleGroup *> & particleGroupsById() const;

private:
    ParticleGroupTycoon();
    ~ParticleGroupTycoon();

    void checkCollisions(double deltaTime);
    std::shared_ptr<ParticleCollision> m_collisions;
    double m_collisionCheckDelta;

    void splitGroups();
    void mergeGroups();
    double m_timeSinceSplit;

    uint64_t gridIndexFromPosition(const glm::vec3 & position);
    DownGroup * particleGroupAtGridIndex(uint64_t index, const std::string elementName);
    void insertGroupIntoGrid(DownGroup * group, uint64_t index);

    static ParticleGroupTycoon * s_instance;

    std::unordered_map<unsigned int, ParticleGroup *> m_particleGroups;
    std::unordered_map<std::string, std::unordered_map<uint64_t, DownGroup *> > m_grid;
};
