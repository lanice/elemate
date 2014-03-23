#include "physicswrapper.h"

#include <iostream>
#include <cassert>

#include <glow/logging.h>

#include "utils/pxcompilerfix.h"
#include <PxPhysicsAPI.h>

#include "physicserrorcallback.h"
#include "elements.h"
#include "particles/particlescriptaccess.h"


const int   PhysicsWrapper::kNumberOfThreads = 4;
PhysicsWrapper * PhysicsWrapper::s_instance = nullptr;

PhysicsWrapper::PhysicsWrapper()
: m_foundation(nullptr)
, m_cpu_dispatcher(nullptr)
, m_physics(nullptr)
, m_scene(nullptr)
, m_physxGpuAvailable(checkPhysxGpuAvailable())
, m_cudaContextManager(nullptr)
, m_gpuParticles(false)
{
    initializePhysics();
    initializeScene();
    Elements::initialize();

    s_instance = this;
}

PhysicsWrapper::~PhysicsWrapper()
{
    s_instance = nullptr;

    Elements::clear();

    m_scene->fetchResults(); //Wait for last simulation step to complete before releasing scene
    m_scene->release();
    m_cpu_dispatcher->release();
    m_physics->release();
    PxCloseExtensions();
    if (m_cudaContextManager)
        m_cudaContextManager->release();
    m_foundation->release();
}

bool PhysicsWrapper::checkPhysxGpuAvailable()
{
#ifdef PX_WINDOWS
    bool gpuPhysx = -1 != physx::PxGetSuggestedCudaDeviceOrdinal(m_errorCallback);
#else
    bool gpuPhysx = false;
#endif
    if (gpuPhysx)
        glow::info("PhysX GPU acceleration is available. Press G to toggle this feature.");
    else
        glow::info("PhysX GPU acceleration is not available. Calculating the particles on the CPU.");
    return gpuPhysx;
}

bool PhysicsWrapper::physxGpuAvailable()
{
    return getInstance()->m_physxGpuAvailable;
}

PhysicsWrapper * PhysicsWrapper::getInstance()
{
    assert(s_instance);
    return s_instance;
}

void PhysicsWrapper::step(double delta)
{
    if (delta == 0)
        return;

    m_scene->simulate(static_cast<physx::PxReal>(delta));
    m_scene->fetchResults(true);
}

void PhysicsWrapper::initializePhysics(){
    static physx::PxDefaultAllocator     gDefaultAllocatorCallback;

    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, m_errorCallback);
    if (!m_foundation)
        fatalError("PxCreateFoundation failed!");
    m_physics= PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale());
    if (!m_physics)
        fatalError("PxCreatePhysics failed!");

#ifdef PX_WINDOWS
    if (m_physxGpuAvailable) {
        // create cuda context manager
        physx::PxCudaContextManagerDesc cudaContextManagerDesc;
        m_cudaContextManager = physx::PxCreateCudaContextManager(*m_foundation, cudaContextManagerDesc, nullptr);
    }
#endif

    //    Extensions
    // The extensions library contains many functions that may be useful to a large class of users, 
    // but which some users may prefer to omit from their application either for code size reasons or 
    // to avoid use of certain subsystems, such as those pertaining to networking. 
    // Initializing the extensions library requires the PxPhysics object:
    if (!PxInitExtensions(*m_physics))
        fatalError("PxInitExtensions failed!");
}

void PhysicsWrapper::initializeScene(){
    physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
    customizeSceneDescription(sceneDesc);

    if (!sceneDesc.cpuDispatcher)
    {
        m_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(kNumberOfThreads);
        if (!m_cpu_dispatcher)
            fatalError("PxDefaultCpuDispatcherCreate failed!");
        sceneDesc.cpuDispatcher = m_cpu_dispatcher;
    }
    if (!sceneDesc.filterShader)
        sceneDesc.filterShader = &physx::PxDefaultSimulationFilterShader;

#ifdef PX_WINDOWS
    if (m_cudaContextManager)
        sceneDesc.gpuDispatcher = m_cudaContextManager->getGpuDispatcher();
#endif

    m_scene = m_physics->createScene(sceneDesc);
    if (!m_scene)
        fatalError("createScene failed!");
}

void PhysicsWrapper::customizeSceneDescription(physx::PxSceneDesc& scene_description){
    scene_description.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
}

void PhysicsWrapper::fatalError(const std::string & error_message){
    glow::fatal("PhysX Error occured:\n%;\nPress Enter to close the Application.", error_message);
    std::getc(stdin);
    exit(1);
}

physx::PxScene* PhysicsWrapper::scene() const
{
    assert(m_scene);
    return m_scene;
}

physx::PxCudaContextManager * PhysicsWrapper::cudaContextManager() const
{
    assert(m_cudaContextManager);
    return m_cudaContextManager;
}

void PhysicsWrapper::addActor(physx::PxActor & actor)
{
    scene()->addActor(actor);
}

void PhysicsWrapper::addActor(physx::PxRigidStatic & actor)
{
    scene()->addActor(actor);
}

void PhysicsWrapper::setUseGpuParticles(bool useGPU)
{
    if (!m_physxGpuAvailable) {
        glow::warning("PhysX calculation on GPU not available!");
        return;
    }
    m_gpuParticles = useGPU;
    ParticleScriptAccess::instance().setUseGpuParticles(useGPU);
}

void PhysicsWrapper::toogleUseGpuParticles()
{
    if (!m_physxGpuAvailable) {
        glow::warning("PhysX calculation on GPU not available!");
        return;
    }
    if (!m_gpuParticles)
        glow::info("Enabling particle simulation on GPU...");
    else
        glow::info("Disabling particle simulation on GPU...");
    setUseGpuParticles(!m_gpuParticles);
}

bool PhysicsWrapper::useGpuParticles() const
{
    if (!m_physxGpuAvailable) {
        return false;
    }
    return m_gpuParticles;
}

void PhysicsWrapper::pauseGPUAcceleration()
{
    if (!m_physxGpuAvailable) {
        return;
    }
    ParticleScriptAccess::instance().pauseGPUAcceleration();
}

void PhysicsWrapper::restoreGPUAccelerated()
{
    if (!m_physxGpuAvailable) {
        return;
    }
    ParticleScriptAccess::instance().restoreGPUAccelerated();
}
