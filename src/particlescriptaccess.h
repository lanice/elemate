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

    /** Must be called from outside to initialize the LuaWrapper instance. */
    void init();

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

    void setMaxMotionDistance(int index, float maxMotionDistance);
    void setGridSize(int index, float gridSize);
    void setRestOffset(int index, float restOffset);
    void setContactOffset(int index, float contactOffset);
    void setRestParticleDistance(int index, float restParticleDistance);
    void setRestitution(int index, float restitution);
    void setDynamicFriction(int index, float dynamicFriction);
    void setStaticFriction(int index, float staticFriction);
    void setDamping(int index, float damping);
    void setParticleMass(int index, float particleMass);
    void setViscosity(int index, float viscosity);
    void setStiffness(int index, float stiffness);

    float maxMotionDistance(int index);
    float gridSize(int index);
    float restOffset(int index);
    float contactOffset(int index);
    float restParticleDistance(int index);
    float restitution(int index);
    float dynamicFriction(int index);
    float staticFriction(int index);
    float damping(int index);
    float particleMass(int index);
    float viscosity(int index);
    float stiffness(int index);
    /************************************************/    

    void setUpParticleGroup(const int index, const std::string & elementType);

    std::vector<std::tuple<ParticleGroup *, std::string> > m_particleGroups;
    std::vector<int> m_freeIndices;

    World * m_worldNotifier;

    bool m_gpuParticles;
    uint8_t m_gpuParticlesPauseFlags;

    LuaWrapper * m_lua;

    static ParticleScriptAccess s_access;
};
