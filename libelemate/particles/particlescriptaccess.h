#pragma once

#include <string>
#include <map>
#include <utility>
#include <inttypes.h>
#include <memory>

namespace physx { class PxScene; }
class ParticleGroup;
class ParticleCollision;
class World;
class LuaWrapper;


/**
 * Provides global access to ParticleGroups.
 */
class ParticleScriptAccess
{
public:
    /** Must be called from outside to initialize the global instance and its LuaWrapper. */
    static void initialize(World & notifier);
    static void release();
    static ParticleScriptAccess& instance();

    ParticleGroup * particleGroup(const int id);

    /** Creates an instance of ParticleGroup and registers it, returning the access id */
    int createParticleGroup(bool emittingGroup, const std::string & elementType = "default", uint32_t maxParticleCount = 10000U);
    void removeParticleGroup(const int id);
    void clearParticleGroups();

    void setNotifier(World * notifier);

    /** enable/disable GPU acceleration. */
    void setUseGpuParticles(const bool enable);
    /** pause the gpu acceleration if enabled, for scene mesh updates */
    void pauseGPUAcceleration();
    /** restart gpu acceleration if it was enabled before last call of pauseGPUAcceleration */
    void restoreGPUAccelerated();

    void registerLuaFunctions(LuaWrapper & lua);

    void checkCollisions(double deltaTime);


protected:
    ParticleScriptAccess();
    ~ParticleScriptAccess();
    static ParticleScriptAccess * s_instance;

    /** Functions callable from within lua scripts. */
    void createParticle(const int id, const float positionX, const float positionY, const float positionZ, const float velocityX, const float velocityY, const float velocityZ);
    void emit(const int id, const float ratio, const float positionX, const float positionY, const float positionZ, const float directionX, const float directionY, const float directionZ);
    void stopEmit(const int id);
    void setImmutableProperties( const int id, const float maxMotionDistance, const float gridSize, const float restOffset, const float contactOffset, const float restParticleDistance);
    void setMutableProperties(const int id, const float restitution, const float dynamicFriction, const float staticFriction, const float damping, const float particleMass, const float viscosity, const float stiffness);
    int numParticleGroups();
    const std::string & elementAtId(int id);
    int nextParticleGroup(int id);

    void setMaxMotionDistance(int id, float maxMotionDistance);
    void setGridSize(int id, float gridSize);
    void setRestOffset(int id, float restOffset);
    void setContactOffset(int id, float contactOffset);
    void setRestParticleDistance(int id, float restParticleDistance);
    void setRestitution(int id, float restitution);
    void setDynamicFriction(int id, float dynamicFriction);
    void setStaticFriction(int id, float staticFriction);
    void setDamping(int id, float damping);
    void setParticleMass(int id, float particleMass);
    void setViscosity(int id, float viscosity);
    void setStiffness(int id, float stiffness);

    float maxMotionDistance(int id);
    float gridSize(int id);
    float restOffset(int id);
    float contactOffset(int id);
    float restParticleDistance(int id);
    float restitution(int id);
    float dynamicFriction(int id);
    float staticFriction(int id);
    float damping(int id);
    float particleMass(int id);
    float viscosity(int id);
    float stiffness(int id);
    /************************************************/    

    void setUpParticleGroup(const int id, const std::string & elementType);

    std::map<int, ParticleGroup *> m_particleGroups;
    int m_id;

    std::shared_ptr<ParticleCollision> m_collisions;
    double m_collisionCheckDelta;

    World * m_worldNotifier;

    bool m_gpuParticles;
    uint8_t m_gpuParticlesPauseFlags;

    LuaWrapper * m_lua;

    physx::PxScene * m_pxScene;

    friend class ParticleCollision;
};
