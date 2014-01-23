#include "particlescriptaccess.h"

#include "particlegroup.h"
#include "lua/luawrapper.h"


ParticleScriptAccess ParticleScriptAccess::s_access;

ParticleScriptAccess::ParticleScriptAccess()
{
}

ParticleScriptAccess::~ParticleScriptAccess()
{
}

ParticleScriptAccess& ParticleScriptAccess::instance()
{
    return s_access;
}

int ParticleScriptAccess::createParticleGroup(const std::string & /*elementType*/)
{
    ParticleGroup * particleGroup = new ParticleGroup();
    LuaWrapper * wrapper = new LuaWrapper();

    if (m_freeIndices.size() > 0)
    {
        int i = m_freeIndices.back();
        m_freeIndices.pop_back();
        m_particleGroups.at(i) = std::make_tuple(particleGroup, wrapper);
        return i;
    }

    m_particleGroups.push_back(std::make_tuple(particleGroup, wrapper));
    return m_particleGroups.size() - 1;
}

int ParticleScriptAccess::removeParticleGroup(const int index)
{
    delete std::get<0>(m_particleGroups.at(index));
    delete std::get<1>(m_particleGroups.at(index));

    std::get<0>(m_particleGroups.at(index)) = nullptr;
    std::get<1>(m_particleGroups.at(index)) = nullptr;

    m_freeIndices.push_back(index);
    return 0;
}

void ParticleScriptAccess::updateVisuals()
{
    for (auto particleGroup : m_particleGroups)
    {
        if (std::get<0>(particleGroup)) std::get<0>(particleGroup)->updateVisuals();
    }
}
