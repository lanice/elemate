#pragma once

#include <unordered_map>

class ParticleGroup;

class ParticleGroupTycoon
{
public:
    static void initialize();
    static void release();

    static ParticleGroupTycoon & instance();

    void updatePhysics(double delta);
    void updateVisuals();

private:
    ParticleGroupTycoon();
    ~ParticleGroupTycoon();

    static ParticleGroupTycoon * s_instance;

    std::unordered_map<unsigned int, ParticleGroup *> m_particleGroups;
};
