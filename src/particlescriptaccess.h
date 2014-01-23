#pragma once

#include <string>
#include <vector>


class ParticleGroup;

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

    /** Called automatically to update the ParticleDrawable(s) */
    void updateVisuals();

protected:
    ParticleScriptAccess();

    std::vector<ParticleGroup *> m_particleGroups;
    std::vector<int> m_freeIndices;

    static ParticleScriptAccess s_access;
};
