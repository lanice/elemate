#include "physicswrapper.h"

#include "hpicgs\CyclicTime.h"

// Standard Libs
#include <iostream>
#include "standardparticles.h"
#include "nullobjectplaceholder.h"
#include "apex/elemateresourcecallback.h"
#include "apex/elematerenderresourcemanager.h"

#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "ApexFramework_x64.lib")

const int	PhysicsWrapper::kNumberOfThreads = 2;

PhysicsWrapper::PhysicsWrapper():
		m_foundation(nullptr),
		m_physics(nullptr),
		//m_profile_zone_manager(nullptr),
        m_cyclic_time(nullptr),
		m_scene(nullptr),
		m_cpu_dispatcher(nullptr),
		m_elapsed(0.0f)
{
	initializePhysics();
	initializeScene();
    initializeTime();
    initializeApex();
}

PhysicsWrapper::~PhysicsWrapper(){
	shutdown();
    if (m_cyclic_time)
        delete m_cyclic_time;
}

void PhysicsWrapper::initializePhysics(){
	static physx::PxDefaultErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator	 gDefaultAllocatorCallback;

	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!m_foundation)
		fatalError("PxCreateFoundation failed!");
	m_physics= PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale());
	if (!m_physics)
		fatalError("PxCreatePhysics failed!");

	/*	... we still have to think about those:
	//For Debugging Lab ....
	m_profile_zone_manager = &physx::PxProfileZoneManager::createProfileZoneManager(m_foundation);
	if (!m_profile_zone_manager)
		fatalError("PxProfileZoneManager::createProfileZoneManager failed!");
	*/
	//	  Cooking
	// The PhysX cooking library provides utilities for creating, converting, and serializing bulk data. 
	// Depending on your application, you may wish to link to the cooking library in order to process such data at runtime. 
	// Alternatively you may be able to process all such data in advance and just load it into memory as required. 
	// Initialize the cooking library as follows: (after declaring member physx::PxCooking* m_cooking)

	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, physx::PxCookingParams());
	if (!m_cooking)
		fatalError("PxCreateCooking failed!");

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

    m_scene = m_physics->createScene(sceneDesc);
    if (!m_scene)
        fatalError("createScene failed!");

    m_materials["default"] = m_physics->createMaterial(0.5f, 0.5f, 0.1f);
    if (!m_materials["default"])
        fatalError("createMaterial failed!");
}

void PhysicsWrapper::initializeTime(){
    if (m_cyclic_time)
        delete m_cyclic_time;
    m_cyclic_time = new CyclicTime(0.0L, 1.0L);
}

void PhysicsWrapper::initializeApex(){
    static physx::PxDefaultErrorCallback gDefaultErrorCallback;
    physx::apex::ElemateResourceCallback* rcallback = new physx::apex::ElemateResourceCallback();
    physx::apex::NxApexSDKDesc   apexDesc;
    apexDesc.outputStream = &gDefaultErrorCallback;
    apexDesc.resourceCallback = rcallback;
    apexDesc.physXSDK = m_physics;
    apexDesc.cooking = m_cooking;

    m_render_resource_manager = new physx::apex::ElemateRenderResourceManager(*m_osgGraphicsContext.get());
    apexDesc.renderResourceManager = m_render_resource_manager;

    if (apexDesc.isValid())
        m_apex_sdk = NxCreateApexSDK(apexDesc);
    else
        return;

    rcallback->setApexSDK(m_apex_sdk);

    physx::apex::NxApexSceneDesc apexSceneDesc;
    apexSceneDesc.scene = m_scene;
    m_apex_scene = m_apex_sdk->createScene(apexSceneDesc);


    static const physx::PxU32 viewIDlookAtRightHand = m_apex_scene->allocViewMatrix(physx::apex::ViewMatrixType::LOOK_AT_RH);
    static const physx::PxU32 projIDperspectiveCubicRightHand = m_apex_scene->allocProjMatrix(physx::apex::ProjMatrixType::USER_CUSTOMIZED);
    
    m_standard_particles = new StandardParticles();
    m_standard_particles->initialize(m_apex_sdk);
    m_standard_particles->createEmitter(m_apex_sdk, m_apex_scene);
}

void PhysicsWrapper::customizeSceneDescription(physx::PxSceneDesc& scene_description){
    scene_description.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
}

bool PhysicsWrapper::step(){
    static t_longf now = m_cyclic_time->getNonModf();
    static t_longf last_time = m_cyclic_time->getNonModf();
    
    now = m_cyclic_time->getNonModf(true);
    m_elapsed = now - last_time;
    last_time = now;

    //Simulating would fail with Nullobjects
    //m_apex_scene->simulate(static_cast<physx::PxReal>(elapsedTime()));
    m_scene->simulate(static_cast<physx::PxReal>(elapsedTime()));
	return true;
}

void PhysicsWrapper::shutdown(){
    // This is absolutely necessary to free the scene without conflicts.
    m_apex_scene->setPhysXScene(0);
    m_apex_scene->fetchResults(true, NULL);
    m_apex_scene->release();
    
	m_scene->fetchResults(); //Wait for last simulation step to complete before releasing scene
    m_scene->release();
    m_cpu_dispatcher->release();
	m_physics->release();
	//Please don't forget if you activate this feature.
	//m_profile_zone_manager->release();
	m_foundation->release();
}

void PhysicsWrapper::fatalError(std::string error_message){
	std::cerr << "PhysX Error occured:" << std::endl;
	std::cerr << error_message << std::endl;
	std::cerr << "Press Enter to close the Application" << std::endl;
	std::string temp;
	std::getline(std::cin, temp);
	exit(1);
}

void PhysicsWrapper::startSimulation(){
    m_cyclic_time->start();
}

void PhysicsWrapper::pauseSimulation(){
    m_cyclic_time->pause();
}

void PhysicsWrapper::stopSimulation(){
    m_cyclic_time->stop();
    m_cyclic_time->reset();
}


t_longf PhysicsWrapper::elapsedTime()const{
    return m_elapsed;
}

physx::PxScene*	PhysicsWrapper::scene()const{
    return m_scene;
}

physx::NxApexScene*	PhysicsWrapper::apex_scene()const{
    return m_apex_scene;
}

physx::PxMaterial*	PhysicsWrapper::material(std::string material_name)const{
	return m_materials.at(material_name);
}

void PhysicsWrapper::setOsgGraphicsContext(osg::GraphicsContext * context)
{
    m_osgGraphicsContext = context;
}