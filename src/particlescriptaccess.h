#pragma once

#include <string>
#include <vector>
#include <tuple>


class ParticleGroup;
class LuaWrapper;

/**
 * Provides global access to ParticleGroups.
 */
class ParticleScriptAccess
{
public:
    static ParticleScriptAccess& instance();

    ~ParticleScriptAccess();

    /** Creates an instance of ParticleGroup and registers it, returning the access index */
    int createParticleGroup(const std::string & elementType = "default");
    int removeParticleGroup(const int index);

    /** Called automatically to update the ParticleDrawable(s) */
    void updateVisuals();

protected:
    ParticleScriptAccess();

    std::vector<std::tuple<ParticleGroup *, LuaWrapper *> > m_particleGroups;
    std::vector<int> m_freeIndices;

    static ParticleScriptAccess s_access;
};
