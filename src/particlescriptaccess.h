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

protected:
    ParticleScriptAccess();

    void setUpParticleGroup(ParticleGroup * particleGroup, LuaWrapper * wrapper, const std::string & elementType);

    std::vector<std::tuple<ParticleGroup *, LuaWrapper *> > m_particleGroups;
    std::vector<int> m_freeIndices;

    World * m_worldNotifier;

    static ParticleScriptAccess s_access;
};
