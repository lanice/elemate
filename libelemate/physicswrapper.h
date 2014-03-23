#pragma once

#include <string>
#include "physicserrorcallback.h"

namespace physx {
    class PxPhysics;
    class PxFoundation;
    class PxScene;
    class PxSceneDesc;
    class PxDefaultCpuDispatcher;
    class PxActor;
    class PxRigidStatic;
    class PxCudaContextManager;
}

/** Wraps the NVIDIA PhysX context and scene. Allows use of CUDA accelerated particles on windows and supporting NVIDIA GPUs. */
class PhysicsWrapper
{
public:
    PhysicsWrapper();
    ~PhysicsWrapper();

    /** @return the current PhysicsWrapper object */
    static PhysicsWrapper * getInstance();

    /** Proceeds with simulation for a given time delta. Calls simulate and a blocking fetchResults on the PhysX scene.
        @param delta floating point time in seconds */
    void step(float delta);
    
    /** @returns the PhysX scene */
    physx::PxScene * scene() const;

    /** @returns the CUDA context manager on Windows, if a supporting GPU was found.  */
    physx::PxCudaContextManager * cudaContextManager() const;

    void setUseGpuParticles(bool useGPU);
    void toogleUseGpuParticles();
    bool useGpuParticles() const;
    /** pause the GPU acceleration if currently enabled. Needed for scene mesh updates */
    void pauseGPUAcceleration();
    /** restart GPU acceleration if it was enabled before last call of pauseGPUAcceleration */
    void restoreGPUAccelerated();

    static bool physxGpuAvailable();

private:
    /** Number of threads is required for the CPU Dispatcher of th PhysX library. */
    static const int kNumberOfThreads;

    /** Creation of PxFoundation, PxPhysics and Initialization of PxExtensions. */
    void initializePhysics();

    /** Creation of PxDefaultCpuDispatcher and PxScene. */
    void initializeScene();

    /** Specifies special scene description. */
    void customizeSceneDescription(physx::PxSceneDesc&);

    /** Prints an error message and end the application after pressing enter. */
    void fatalError(const std::string & error_message);

    bool checkPhysxGpuAvailable();
    
    PhysicsErrorCallback                            m_errorCallback;
    physx::PxFoundation*                            m_foundation;
    physx::PxDefaultCpuDispatcher*                  m_cpu_dispatcher;
    physx::PxPhysics*                               m_physics;
    physx::PxScene*                                 m_scene;
    const bool                                      m_physxGpuAvailable;
    physx::PxCudaContextManager*                    m_cudaContextManager;

    bool                                            m_gpuParticles;
    
    static PhysicsWrapper * s_instance;

public:
    PhysicsWrapper(PhysicsWrapper&) = delete;
    void operator=(PhysicsWrapper&) = delete;
};
