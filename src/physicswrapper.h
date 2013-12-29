#pragma once

#include <string>
#include <list>
#include <memory>

#include <foundation/PxVec3.h>
#include <foundation/PxSimpleTypes.h>

namespace physx {
    class PxPhysics;
    class PxFoundation;
    class PxScene;
    class PxSceneDesc;
    class PxDefaultCpuDispatcher;
    class PxCooking;
    class PxProfileZoneManager;
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

    /** Proceeds with simulation for amount of given time delta. */
    bool step(long double delta);
    
    /** Creates a particle emitter */
    void makeParticleEmitter(const physx::PxVec3& position);

    /** Creates a ball. */
    void makeStandardBall(const physx::PxVec3& global_position, physx::PxReal radius, const physx::PxVec3& linear_velocity, const physx::PxVec3& angular_velocity);


    /** The returned object is initialized. */
    physx::PxScene*             scene() const;

protected:
    /** Default value is 2. Number of threads is required for the CPU Dispatcher of th PhysX library. */
    static const int            kNumberOfThreads;

    /** Sets and rotates every object according to its representation in PhysX. */
    void updateAllObjects(long double delta);

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

    std::list<std::shared_ptr<ParticleEmitter>>     m_emitters;

public:
    PhysicsWrapper(PhysicsWrapper&) = delete;
    void operator=(PhysicsWrapper&) = delete;
};