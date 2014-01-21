#include "physicswrapper.h"

#include <iostream>
#include <cassert>

#include <glow/logging.h>

#include "pxcompilerfix.h"
#include <PxPhysicsAPI.h>

#include "elements.h"
#include "particleemitter.h"
#include "lua/luawrapper.h"


const int   PhysicsWrapper::kNumberOfThreads = 2;
PhysicsWrapper * PhysicsWrapper::s_instance = nullptr;

PhysicsWrapper::PhysicsWrapper()
: m_foundation(nullptr)
, m_cpu_dispatcher(nullptr)
, m_physics(nullptr)
, m_scene(nullptr)
, m_physxGpuAvailable(checkPhysxGpuAvailable())
, m_cudaContextManager(nullptr)
, m_emitters()
, m_lua(new LuaWrapper())
, m_activeEmitter("")
, m_gpuParticles(false)
//m_profile_zone_manager(nullptr)
{
    initializePhysics();
    initializeScene();
    Elements::initialize(*m_physics);

    s_instance = this;
}

PhysicsWrapper::~PhysicsWrapper()
{
    s_instance = nullptr;

    Elements::clear();

    clearEmitters();

    m_scene->fetchResults(); //Wait for last simulation step to complete before releasing scene
    m_scene->release();
    m_cpu_dispatcher->release();
    m_physics->release();
    PxCloseExtensions();
    //Please don't forget if you activate this feature.
    //m_profile_zone_manager->release();
    if (m_cudaContextManager)
        m_cudaContextManager->release();
    m_foundation->release();

    delete m_lua;
}

bool PhysicsWrapper::checkPhysxGpuAvailable()
{
#ifdef PX_WINDOWS
    bool gpuPhysx = -1 != physx::PxGetSuggestedCudaDeviceOrdinal(m_errorCallback);
#elif
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

    for (auto& emitter : m_emitters){
        emitter.second->step(delta);
    }
}

void PhysicsWrapper::updateAllObjects()
{
    for (auto& emitter : m_emitters){
        emitter.second->update();
    }
}

void PhysicsWrapper::makeParticleEmitter(const std::string& emitter_name, const glm::vec3& position){
    m_emitters.emplace(emitter_name, new ParticleEmitter(
        m_gpuParticles,
        physx::PxVec3(position.x, position.y, position.z)));
    m_emitters[emitter_name]->initializeParticleSystem(Elements::emitterDescription(emitter_name));
}

void PhysicsWrapper::clearEmitters(){
    for (auto emitter : m_emitters)
        delete emitter.second;
    m_emitters.clear();
}

void PhysicsWrapper::initializePhysics(){
    static physx::PxDefaultErrorCallback gDefaultErrorCallback;
    static physx::PxDefaultAllocator     gDefaultAllocatorCallback;

    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
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

    /* ... we still have to think about those:
    //For Debugging Lab ....
    m_profile_zone_manager = &physx::PxProfileZoneManager::createProfileZoneManager(m_foundation);
    if (!m_profile_zone_manager)
        fatalError("PxProfileZoneManager::createProfileZoneManager failed!");
    
    // Cooking
    // The PhysX cooking library provides utilities for creating, converting, and serializing bulk data. 
    // Depending on your application, you may wish to link to the cooking library in order to process such data at runtime. 
    // Alternatively you may be able to process all such data in advance and just load it into memory as required. 
    // Initialize the cooking library as follows: (after declaring member physx::PxCooking* m_cooking)

    m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, PxCookingParams());
    if (!m_cooking)
        fatalError("PxCreateCooking failed!");
    */

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

void PhysicsWrapper::fatalError(std::string error_message){
    glow::fatal("PhysX Error occured:\n%;\nPress Enter to close the Application.", error_message);
    std::string temp;
    std::getline(std::cin, temp);
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
    for (auto emitter : m_emitters)
        emitter.second->setGPUAccelerated(m_gpuParticles);
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
        glow::warning("PhysX calculation on GPU not available!");
        return false;
    }
    return m_gpuParticles;
}

void PhysicsWrapper::pauseGPUAcceleration()
{
    if (!m_physxGpuAvailable) {
        glow::warning("PhysX calculation on GPU not available!");
        return;
    }
    for (auto emitter : m_emitters)
        emitter.second->pauseGPUAcceleration();
}

void PhysicsWrapper::restoreGPUAccelerated()
{
    if (!m_physxGpuAvailable) {
        glow::warning("PhysX calculation on GPU not available!");
        return;
    }
    for (auto emitter : m_emitters)
        emitter.second->restoreGPUAccelerated();
}

void PhysicsWrapper::updateEmitterPosition(const glm::vec3& position)
{
    if (m_activeEmitter != "")
        m_emitters[m_activeEmitter]->setPosition(physx::PxVec3(position.x, position.y, position.z));
}

void PhysicsWrapper::selectEmitter(const std::string& emitter_name)
{
    physx::PxVec3 current_hand_position = physx::PxVec3(0.0F, 0.0F, 0.0F);
    if (m_activeEmitter != ""){
        current_hand_position = m_emitters[m_activeEmitter]->position();
        stopEmitting();
    }
    m_activeEmitter = emitter_name;
    m_emitters[m_activeEmitter]->setPosition(current_hand_position);
}

void PhysicsWrapper::startEmitting()
{
    if (m_activeEmitter != "")
        m_emitters[m_activeEmitter]->startEmit();
}

void PhysicsWrapper::stopEmitting()
{
    if (m_activeEmitter != "")
        m_emitters[m_activeEmitter]->stopEmit();
}
void PhysicsWrapper::reloadLua()
{
    m_lua->reloadScripts();
}

void ElematePxErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
{
    switch (code) {
    case physx::PxErrorCode::eNO_ERROR:
        glow::info("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eDEBUG_INFO:
        glow::debug("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eDEBUG_WARNING:
        glow::warning("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eINVALID_PARAMETER:
    case physx::PxErrorCode::eINVALID_OPERATION:
    case physx::PxErrorCode::eOUT_OF_MEMORY:
        glow::critical("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eINTERNAL_ERROR:
        glow::fatal("PhysX: ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eABORT:
        glow::fatal("PhysX: ""%;"" [%;%;]", message, file, line);
        exit(2);
    case physx::PxErrorCode::ePERF_WARNING:
        glow::debug("PhysX (performance): ""%;"" [%;%;]", message, file, line);
        break;
    case physx::PxErrorCode::eMASK_ALL:
        glow::fatal("PhysX (something terrible happened): ""%;"" [%;%;]", message, file, line);
        break;
    }
}
