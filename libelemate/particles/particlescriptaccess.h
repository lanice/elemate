#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <inttypes.h>
#include <memory>

#include <glm/glm.hpp>

namespace physx { class PxScene; }
class ParticleGroup;
class LuaWrapper;


/**
 * Provides global access to ParticleGroups.
 */
class ParticleScriptAccess
{
public:
    /** Must be called from outside to initialize the global instance and its LuaWrapper. */
    static void initialize(std::unordered_map<unsigned int, ParticleGroup*> & particleGroups);
    static void release();
    static ParticleScriptAccess& instance();

    /** Creates an instance of ParticleGroup and registers it, returning the access id */
    int createParticleGroup(bool emittingGroup, const std::string & elementType = "default", uint32_t maxParticleCount = 10000U);
    /** configures physical parameters of a ParticleGroup with lua script of given elementType. */
    void setUpParticleGroup(const int id, const std::string & elementType);

    int addParticleGroup(ParticleGroup * group);
    void removeParticleGroup(const int id);
    void clearParticleGroups();

    /** enable/disable GPU acceleration. */
    void setUseGpuParticles(const bool enable);
    /** pause the gpu acceleration if enabled, for scene mesh updates */
    void pauseGPUAcceleration();
    /** restart gpu acceleration if it was enabled before last call of pauseGPUAcceleration */
    void restoreGPUAccelerated();

    /** Registers functions of the ParticleScriptAccess that can be used within given LuaWrapper instance. */
    void registerLuaFunctions(LuaWrapper & lua);


protected:
    friend class ParticleGroupTycoon;

    ParticleScriptAccess(std::unordered_map<unsigned int, ParticleGroup*> & particleGroups);
    ~ParticleScriptAccess();
    static ParticleScriptAccess * s_instance;

    ParticleGroup * particleGroup(const int id);


    /** Callable from within lua scripts. */
    void createParticle(const int id, const float positionX, const float positionY, const float positionZ, const float velocityX, const float velocityY, const float velocityZ);
    /** Callable from within lua scripts. */
    void emit(const int id, const float ratio, const float positionX, const float positionY, const float positionZ, const float directionX, const float directionY, const float directionZ);
    /** Callable from within lua scripts. */
    void stopEmit(const int id);
    /** Callable from within lua scripts. */
    void setImmutableProperties( const int id, const float maxMotionDistance, const float gridSize, const float restOffset, const float contactOffset, const float restParticleDistance);
    /** Callable from within lua scripts. */
    void setMutableProperties(const int id, const float restitution, const float dynamicFriction, const float staticFriction, const float damping, const glm::vec3 &externalAcceleration, const float particleMass, const float viscosity, const float stiffness);
    int numParticleGroups();
    /** Callable from within lua scripts. */
    const std::string & elementAtId(int id);
    /** Callable from within lua scripts. */
    int nextParticleGroup(int id);

    /** Callable from within lua scripts. */
    void setMaxMotionDistance(int id, float maxMotionDistance);
    /** Callable from within lua scripts. */
    void setGridSize(int id, float gridSize);
    /** Callable from within lua scripts. */
    void setRestOffset(int id, float restOffset);
    /** Callable from within lua scripts. */
    void setContactOffset(int id, float contactOffset);
    /** Callable from within lua scripts. */
    void setRestParticleDistance(int id, float restParticleDistance);
    /** Callable from within lua scripts. */
    void setRestitution(int id, float restitution);
    /** Callable from within lua scripts. */
    void setDynamicFriction(int id, float dynamicFriction);
    /** Callable from within lua scripts. */
    void setStaticFriction(int id, float staticFriction);
    /** Callable from within lua scripts. */
    void setDamping(int id, float damping);
    /** Callable from within lua scripts. */
    void setParticleMass(int id, float particleMass);
    /** Callable from within lua scripts. */
    void setViscosity(int id, float viscosity);
    /** Callable from within lua scripts. */
    void setStiffness(int id, float stiffness);
    /** Callable from within lua scripts. */
    void setExternalAcceleration(int id, const glm::vec3 &externalAcceleration);

    /** Callable from within lua scripts. */
    float maxMotionDistance(int id);
    /** Callable from within lua scripts. */
    float gridSize(int id);
    /** Callable from within lua scripts. */
    float restOffset(int id);
    /** Callable from within lua scripts. */
    float contactOffset(int id);
    /** Callable from within lua scripts. */
    float restParticleDistance(int id);
    /** Callable from within lua scripts. */
    float restitution(int id);
    /** Callable from within lua scripts. */
    float dynamicFriction(int id);
    /** Callable from within lua scripts. */
    float staticFriction(int id);
    /** Callable from within lua scripts. */
    float damping(int id);
    /** Callable from within lua scripts. */
    float particleMass(int id);
    /** Callable from within lua scripts. */
    float viscosity(int id);
    /** Callable from within lua scripts. */
    float stiffness(int id);
    /** Callable from within lua scripts. */
    glm::vec3 externalAcceleration(int id);

    std::unordered_map<unsigned int, ParticleGroup *> & m_particleGroups;
    unsigned int m_id;

    bool m_gpuParticles;
    uint8_t m_gpuParticlesPauseFlags;

    LuaWrapper * m_lua;

    physx::PxScene * m_pxScene;

public:
    void operator=(ParticleScriptAccess&) = delete;
};
