#include "physicswrapper.h"

#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")

PhysicsWrapper::PhysicsWrapper(){
	initializePhysics();
	initializeScene();
}

PhysicsWrapper::~PhysicsWrapper(){
	shutdown();
}

void PhysicsWrapper::initializePhysics(){
	static physx::PxDefaultErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator	 gDefaultAllocatorCallback;

	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!m_foundation)
		fatalError("PxCreateFoundation failed!");
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale());
	if (!m_physics)
		fatalError("PxCreatePhysics failed!");

	/*	... we still have to think about those:
	//For Debugging Lab ....
	m_profile_zone_manager = &physx::PxProfileZoneManager::createProfileZoneManager(m_foundation);
	if (!m_profile_zone_manager)
		fatalError("PxProfileZoneManager::createProfileZoneManager failed!");
	
	//	  Cooking
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
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
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
}

void PhysicsWrapper::customizeSceneDescription(physx::PxSceneDesc& scene_description){
	//Modify the Scene Description 
}

bool PhysicsWrapper::step(physx::PxReal dt){
	m_accumulator += dt;
	if (m_accumulator < m_step_size)
		return false;

	m_accumulator -= m_step_size;

	m_scene->simulate(m_step_size);
	return true;
}

void PhysicsWrapper::shutdown(){
	m_foundation->release();
	m_physics->release();
}

void PhysicsWrapper::fatalError(string error_message){
	std::cerr << "PhysX Error occured:" << std::endl;
	std::cerr << error_message << std::endl;
	std::cerr << "Press Enter to close the Application" << std::endl;
	string temp;
	std::getline(std::cin, temp);
	exit(1);
}