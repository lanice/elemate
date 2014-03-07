#include "particlegrouptycoon.h"

#include <cassert>

#include "particlegroup.h"
#include "particlescriptaccess.h"

ParticleGroupTycoon * ParticleGroupTycoon::s_instance = nullptr;

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
{
    ParticleScriptAccess::initialize(m_particleGroups);
}

ParticleGroupTycoon::~ParticleGroupTycoon()
{
    for (auto pair : m_particleGroups)
        delete pair.second;

    m_particleGroups.clear();
}

void ParticleGroupTycoon::updatePhysics(double delta)
{
    for (auto pair : m_particleGroups)
        pair.second->updatePhysics(delta);
}

void ParticleGroupTycoon::updateVisuals()
{
    for (auto pair : m_particleGroups)
        pair.second->updateVisuals();
}
