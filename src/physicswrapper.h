#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);

typedef long double t_longf;

#include <string>
#include <hash_map>
#include <osg/GraphicsContext>

#include "PxPhysicsAPI.h"

class CyclicTime;

/** This Class initializes all basic objects that are necessary to use NVIDIA Physics.
 * At the moment, the properly initialized physics and scene object are accessible via functions.
 * Using this class is simple: Istantiate it and you are able to use the already initialized physics and scene. To make a step in scene-simulation, call step() with the passed time.
 * We still have to decide if we want to wrap the standard functions of PhysX like addActor and athe creation of rigid actors themselves into a separate function.
 * The PxProfilZoneManager is currently disabled, but required code in Constructor, Destructor and Header file is present.
 */
class PhysicsWrapper{
public:
    /** Explicit Constructor because Copying and Assignments are disabled. */
    explicit PhysicsWrapper();
    ~PhysicsWrapper();
    /** Proceeds with simulation for amount of given time and returns if enough time has passed to do another step. */
    bool step();

    /** Starts CycleTimer and continues at last point if it was stopped before using pauseSimulation. */
    void startSimulation();

    /** Pauses internal timer, so there won't be any progress until continued using startSimulation. */
    void pauseSimulation();

    /** Ends Simulation and resets Timer. */
    void stopSimulation();

    /** Gets Elapsed Time without wasting the cycle time. */
    t_longf elapsedTime()const;

    /** The returned object is initialized. */
    physx::PxScene*             scene() const;
    physx::PxMaterial*          material(std::string material_name)const;

    void setOsgGraphicsContext(osg::GraphicsContext * context);

protected:
    /** Default value is 2. Number of threads is required for the CPU Dispatcher of th PhysX library. */
    static const int            kNumberOfThreads;

    /** Creation of PxFoundation, PxPhysics and Initialization of PxExtensions. */
    void initializePhysics();

    /** Creation of PxDefaultCpuDispatcher and PxScene after customized SceneDescription provided by customizeSceneDescription() */
    void initializeScene();

    /** Creation of CycleTime without starting it. */
    void initializeTime();
    
    /** Specifies special scene description.  */
    void customizeSceneDescription(physx::PxSceneDesc&);

    /** Releases all members to free their instances. Automatically called in Destructor. */
    void shutdown();

    /** Prints an error message and end the application after pressing enter. */
    void fatalError(std::string error_message);

    physx::PxFoundation*                            m_foundation;
    //physx::PxProfileZoneManager*                  m_profile_zone_manager; ///< currently disabled.
    physx::PxDefaultCpuDispatcher*                  m_cpu_dispatcher;
    physx::PxPhysics*                               m_physics;
    physx::PxScene*                                 m_scene;
    std::hash_map<std::string, physx::PxMaterial*>  m_materials;
    CyclicTime*                                     m_cyclic_time;
    t_longf                                         m_elapsed;
    physx::PxCooking*                               m_cooking;

    osg::ref_ptr<osg::GraphicsContext> m_osgGraphicsContext;
private:
    DISALLOW_COPY_AND_ASSIGN(PhysicsWrapper);
};