#pragma once

#include <unordered_map>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

class ParticleGroup;
class DownGroup;
class ParticleCollision;

/** @brief Manages ParticleGroups in terms of collision check timing, splitting an merging. */
class ParticleGroupTycoon
{
public:
    static void initialize();
    static void release();

    static ParticleGroupTycoon & instance();

    /** Invoke timed collison checking, update physics and split/merge particle groups as needed. */
    void updatePhysics(double delta);
    /** Update visuals of all particle of all ParticleGroups. */
    void updateVisuals();

    /** Locate and return the nearest DownGroup of a given element. */
    DownGroup * getNearestGroup(const std::string & elementName, const glm::vec3 & position);

    ParticleGroup * particleGroupById(unsigned int id);
    const ParticleGroup * particleGroupById(unsigned int id) const;

    const std::unordered_map<unsigned int, ParticleGroup *> & particleGroups() const;

private:
    ParticleGroupTycoon();
    ~ParticleGroupTycoon();

    /** Calls ParticleCollision::performCheck() ca twice a second. */
    void checkCollisions(double deltaTime);
    std::shared_ptr<ParticleCollision> m_collisions;
    double m_collisionCheckDelta;

    /** Splits ParticleGroups which particles are too widely spread. */
    void splitGroups();
    /** Merges ParticleGroups which overlap too much. */
    void mergeGroups();
    double m_timeSinceSplit;

    /** Calculates the gridIndex of given position (relevant for merging). */
    uint64_t gridIndexFromPosition(const glm::vec3 & position);
    /** Returns pointer to the ParticleGroup of type elementName which is currently at gridIndex index. returns nullptr if no ParticleGroup of given type is at gridIndex index. */
    DownGroup * particleGroupAtGridIndex(uint64_t index, const std::string elementName);
    /** Assigns a ParticleGroup to a grid index(for merge-checking). */
    void insertGroupIntoGrid(DownGroup * group, uint64_t index);

    static ParticleGroupTycoon * s_instance;

    std::unordered_map<unsigned int, ParticleGroup *> m_particleGroups;
    std::unordered_map<std::string, std::unordered_map<uint64_t, DownGroup *> > m_grid;
};
