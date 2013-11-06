#include "physicswrapper.h"

#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")

PhysicsWrapper::PhysicsWrapper(){
	initialize();
}

PhysicsWrapper::~PhysicsWrapper(){
	shutdown();
}

void PhysicsWrapper::initialize(){
	static physx::PxDefaultErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator gDefaultAllocatorCallback;

	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!m_foundation)
		fatalError("PxCreateFoundation failed!");
	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale());
	if (!m_physics)
		fatalError("PxCreatePhysics failed!");
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