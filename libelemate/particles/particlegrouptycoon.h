#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

class ParticleGroup;

class ParticleGroupTycoon
{
public:
    static void initialize();
    static void release();

    static ParticleGroupTycoon & instance();

    void updatePhysics(double delta);
    void updateVisuals();

    ParticleGroup * getNearestGroup(const std::string & elementName, const glm::vec3 & position);

private:
    ParticleGroupTycoon();
    ~ParticleGroupTycoon();

    void splitGroups();
    double m_timeSinceSplit;

    static ParticleGroupTycoon * s_instance;

    std::unordered_map<unsigned int, ParticleGroup *> m_particleGroups;
};
