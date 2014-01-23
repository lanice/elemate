#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include <glm/glm.hpp>

#include "pxcompilerfix.h"
#include "foundation/PxErrorCallback.h"

namespace physx {
    class PxPhysics;
    class PxFoundation;
    class PxScene;
    class PxSceneDesc;
    class PxDefaultCpuDispatcher;
    class PxCooking;
    class PxProfileZoneManager;
    class PxActor;
    class PxRigidStatic;
    class PxCudaContextManager;
}
class ParticleEmitter;
class LuaWrapper;


class ElematePxErrorCallback : public physx::PxErrorCallback
{
public:
    virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override;
};


/** This Class initializes all basic objects that are necessary to use NVIDIA Physics.
 * At the moment, the properly initialized physics and scene object are accessible via functions.
 * Using this class is simple: Istantiate it and you are able to use the already initialized physics and scene. To make a step in scene-simulation, call step() with the passed time.
 * We still have to decide if we want to wrap the standard functions of PhysX like addActor and athe creation of rigid actors themselves into a separate function.
 * The PxProfilZoneManager is currently disabled, but required code in Constructor, Destructor and Header file is present.
 */
class PhysicsWrapper{
public:
    PhysicsWrapper();
    ~PhysicsWrapper();

    /** return the current PhysicsWrapper object */
    static PhysicsWrapper * getInstance();

    /** Proceeds with simulation for amount of given time delta. */
    void step(double delta);

    /** Sets and rotates every object according to its representation in PhysX. */
    void updateAllObjects();
    
    /** Creates a particle emitter */
    void makeParticleEmitter(const std::string& emitter_name, const glm::vec3& position);
    void updateEmitterPosition(const glm::vec3& position);
    void selectEmitter(const std::string& emitter_name);
    void startEmitting();
    void stopEmitting();
    void clearEmitters();

    void getRestingParticles();

    /** The returned object is initialized. */
    physx::PxScene*             scene() const;

    physx::PxCudaContextManager * cudaContextManager() const;

    /** add the actor to the current physx scene */
    void addActor(physx::PxActor& actor);
    void addActor(physx::PxRigidStatic& actor);

    void reloadLua();

    void setUseGpuParticles(bool useGPU);
    void toogleUseGpuParticles();
    bool useGpuParticles() const;
    /** pause the gpu acceleration if enabled, for scene mesh updates */
    void pauseGPUAcceleration();
    /** restart gpu acceleration if it was enabled before last call of pauseGPUAcceleration */
    void restoreGPUAccelerated();

    static bool physxGpuAvailable();

protected:
    /** Default value is 2. Number of threads is required for the CPU Dispatcher of th PhysX library. */
    static const int            kNumberOfThreads;

    /** Creation of PxFoundation, PxPhysics and Initialization of PxExtensions. */
    void initializePhysics();

    /** Creation of PxDefaultCpuDispatcher and PxScene after customized SceneDescription provided by customizeSceneDescription() */
    void initializeScene();

    /** Specifies special scene description.  */
    void customizeSceneDescription(physx::PxSceneDesc&);

    /** Prints an error message and end the application after pressing enter. */
    void fatalError(std::string error_message);

    bool checkPhysxGpuAvailable();
    

    ElematePxErrorCallback                          m_errorCallback;
    physx::PxFoundation*                            m_foundation;
    //physx::PxProfileZoneManager*                  m_profile_zone_manager; ///< currently disabled.
    physx::PxDefaultCpuDispatcher*                  m_cpu_dispatcher;
    physx::PxPhysics*                               m_physics;
    physx::PxScene*                                 m_scene;
    //physx::PxCooking*                               m_cooking;
    const bool                                      m_physxGpuAvailable;
    physx::PxCudaContextManager*                    m_cudaContextManager;

    std::unordered_map<std::string, ParticleEmitter*>     m_emitters;
    std::string m_activeEmitter;
    bool                                            m_gpuParticles;
    LuaWrapper * m_lua;
    
    static PhysicsWrapper * s_instance;

public:
    PhysicsWrapper(PhysicsWrapper&) = delete;
    void operator=(PhysicsWrapper&) = delete;
};
