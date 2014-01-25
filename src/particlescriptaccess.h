#pragma once

#include <string>
#include <vector>
#include <tuple>



class ParticleGroup;
class World;
class LuaWrapper;


/**
 * Provides global access to ParticleGroups.
 */
class ParticleScriptAccess
{
public:
    static ParticleScriptAccess& instance();

    ~ParticleScriptAccess();

    ParticleGroup * particleGroup(const int index);

    /** Creates an instance of ParticleGroup and registers it, returning the access index */
    int createParticleGroup(const std::string & elementType = "default");
    void removeParticleGroup(const int index);

    void setNotifier(World * notifier);

    /** enable/disable GPU acceleration. */
    void setUseGpuParticles(const bool enable);
    /** pause the gpu acceleration if enabled, for scene mesh updates */
    void pauseGPUAcceleration();
    /** restart gpu acceleration if it was enabled before last call of pauseGPUAcceleration */
    void restoreGPUAccelerated();


protected:
    ParticleScriptAccess();

    void setUpParticleGroup(ParticleGroup * particleGroup, LuaWrapper * wrapper, const std::string & elementType);

    std::vector<std::tuple<ParticleGroup *, LuaWrapper *> > m_particleGroups;
    std::vector<int> m_freeIndices;

    World * m_worldNotifier;

    bool m_gpuParticles;
    uint8_t m_gpuParticlesPauseFlags;

    static ParticleScriptAccess s_access;
};
