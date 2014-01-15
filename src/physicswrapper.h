#pragma once

#include <string>
#include <list>
#include <memory>

#include <glm/glm.hpp>

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
    bool step(double delta);
    
    /** Creates a particle emitter */
    void makeParticleEmitter(const glm::vec3& position);

    /** The returned object is initialized. */
    physx::PxScene*             scene() const;

    physx::PxCudaContextManager * cudaContextManager() const;

    /** add the actor to the current physx scene */
    void addActor(physx::PxActor& actor);
    void addActor(physx::PxRigidStatic& actor);

    void setUseGpuParticles(bool useGPU);
    bool useGpuParticles() const;

protected:
    /** Default value is 2. Number of threads is required for the CPU Dispatcher of th PhysX library. */
    static const int            kNumberOfThreads;

    /** Sets and rotates every object according to its representation in PhysX. */
    void updateAllObjects(double delta);

    /** Creation of PxFoundation, PxPhysics and Initialization of PxExtensions. */
    void initializePhysics();

    /** Creation of PxDefaultCpuDispatcher and PxScene after customized SceneDescription provided by customizeSceneDescription() */
    void initializeScene();

    /** Specifies special scene description.  */
    void customizeSceneDescription(physx::PxSceneDesc&);

    /** Prints an error message and end the application after pressing enter. */
    void fatalError(std::string error_message);
    


    physx::PxFoundation*                            m_foundation;
    //physx::PxProfileZoneManager*                  m_profile_zone_manager; ///< currently disabled.
    physx::PxDefaultCpuDispatcher*                  m_cpu_dispatcher;
    physx::PxPhysics*                               m_physics;
    physx::PxScene*                                 m_scene;
    //physx::PxCooking*                               m_cooking;
    physx::PxCudaContextManager*                    m_cudaContextManager;

    std::list<std::shared_ptr<ParticleEmitter>>     m_emitters;
    bool                                            m_gpuParticles;

    static PhysicsWrapper * s_instance;

public:
    PhysicsWrapper(PhysicsWrapper&) = delete;
    void operator=(PhysicsWrapper&) = delete;
};