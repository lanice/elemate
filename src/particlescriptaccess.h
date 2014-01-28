#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <inttypes.h>




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

    void registerLuaFunctions(LuaWrapper * lua);


protected:
    ParticleScriptAccess();

    /** Functions callable from within lua scripts. */
    void createParticle(const int index, const float positionX, const float positionY, const float positionZ, const float velocityX, const float velocityY, const float velocityZ);
    void emit(const int index, const float ratio, const float positionX, const float positionY, const float positionZ, const float directionX, const float directionY, const float directionZ);
    void stopEmit(const int index);
    void setImmutableProperties( const int index, const float maxMotionDistance, const float gridSize, const float restOffset, const float contactOffset, const float restParticleDistance);
    void setMutableProperties(const int index, const float restitution, const float dynamicFriction, const float staticFriction, const float damping, const float particleMass, const float viscosity, const float stiffness);
    int numParticleGroups();
    std::string elementAtIndex(int index);
    /************************************************/    

    void setUpParticleGroup(ParticleGroup * particleGroup, LuaWrapper * wrapper, const std::string & elementType);

    std::vector<std::tuple<ParticleGroup *, LuaWrapper *, std::string> > m_particleGroups;
    std::vector<int> m_freeIndices;

    World * m_worldNotifier;

    bool m_gpuParticles;
    uint8_t m_gpuParticlesPauseFlags;

    static ParticleScriptAccess s_access;
};
