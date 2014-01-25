#include "particlescriptaccess.h"

#include <cassert>

#include <glow/logging.h>

#include "particlegroup.h"
#include "world.h"
#include "lua/luawrapper.h"



ParticleScriptAccess ParticleScriptAccess::s_access;

ParticleScriptAccess::ParticleScriptAccess()
: m_worldNotifier(nullptr)
, m_gpuParticles(false)
{
}

ParticleScriptAccess::~ParticleScriptAccess()
{
}

ParticleScriptAccess& ParticleScriptAccess::instance()
{
    return s_access;
}

ParticleGroup * ParticleScriptAccess::particleGroup(const int index)
{
    return std::get<0>(m_particleGroups.at(index));
}

int ParticleScriptAccess::createParticleGroup(const std::string & elementType)
{
    ParticleGroup * particleGroup = new ParticleGroup(m_gpuParticles);
    LuaWrapper * wrapper = new LuaWrapper();

    setUpParticleGroup(particleGroup, wrapper, elementType);

    if (m_freeIndices.size() > 0)
    {
        int i = m_freeIndices.back();
        assert(std::get<0>(m_particleGroups.at(i)) == nullptr && std::get<1>(m_particleGroups.at(i)) == nullptr);
        m_freeIndices.pop_back();
        m_particleGroups.at(i) = std::make_tuple(particleGroup, wrapper);
        return i;
    }

    m_particleGroups.push_back(std::make_tuple(particleGroup, wrapper));
    m_worldNotifier->registerObserver(particleGroup);
    return m_particleGroups.size() - 1;
}

void ParticleScriptAccess::removeParticleGroup(const int index)
{
    delete std::get<0>(m_particleGroups.at(index));
    delete std::get<1>(m_particleGroups.at(index));

    std::get<0>(m_particleGroups.at(index)) = nullptr;
    std::get<1>(m_particleGroups.at(index)) = nullptr;

    m_freeIndices.push_back(index);
}

void ParticleScriptAccess::setUpParticleGroup(ParticleGroup * particleGroup, LuaWrapper * wrapper, const std::string & elementType)
{
    std::function<int(float, float, float, float, float)> func1 = [=](float a, float b, float c, float d, float e) {particleGroup->setImmutableProperties(a, b, c, d, e); return 0; };

    std::function<int(float, float, float, float, float, float, float)> func2 = [=](float a, float b, float c, float d, float e, float f, float g) {particleGroup->setMutableProperties(a, b, c, d, e, f, g); return 0; };

    wrapper->Register("particles_setImmutableProperties", func1);
    wrapper->Register("particles_setMutableProperties", func2);

    std::string script = "scripts/" + elementType + ".lua";
    wrapper->loadScript(script);
}

void ParticleScriptAccess::setNotifier(World * notifier)
{
    m_worldNotifier = notifier;
}

void ParticleScriptAccess::setUseGpuParticles(bool enable)
{
    if (m_gpuParticles == enable) return;

    m_gpuParticles = enable;

    for (auto particleGroupTuple : m_particleGroups)
        std::get<0>(particleGroupTuple)->setUseGpuParticles(enable);
}

void ParticleScriptAccess::pauseGPUAcceleration()
{
    assert(m_gpuParticlesPauseFlags == 0x00);
    if ((m_gpuParticlesPauseFlags & 0x01) == 0x01) // break, if already paused
        return;

    m_gpuParticlesPauseFlags = 0x01 | (m_gpuParticles ? 0x10 : 0x00);

    if (!m_gpuParticles)
        return;
    setUseGpuParticles(false);
}

void ParticleScriptAccess::restoreGPUAccelerated()
{
    assert((m_gpuParticlesPauseFlags & 0x01) == 0x01);
    if ((m_gpuParticlesPauseFlags & 0x01) == 0x00)   // break, if not paused
        return;

    if ((m_gpuParticlesPauseFlags & 0x10) == 0x10)
        setUseGpuParticles(true);

    m_gpuParticlesPauseFlags = 0x00;
}
