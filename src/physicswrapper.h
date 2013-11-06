#pragma once

// Standard Libs
#include <string>
#include <iostream>

using std::string;

// NVIDIA PhysX Libs
#include "PxPhysicsAPI.h"


#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
	TypeName(const TypeName&);				\
	void operator=(const TypeName&);	


class PhysicsWrapper{
public:
	explicit PhysicsWrapper();
	~PhysicsWrapper();

	bool step(physx::PxReal dt);

protected:
	static const int kNumberOfThreads = 2;

	void initializePhysics();
	void initializeScene();
	void customizeSceneDescription(physx::PxSceneDesc&);
	void shutdown();
	void fatalError(string error_message);

	physx::PxFoundation*			m_foundation			= nullptr;
	physx::PxPhysics*				m_physics				= nullptr;
	physx::PxProfileZoneManager*	m_profile_zone_manager	= nullptr;
	physx::PxScene*					m_scene					= nullptr;
	physx::PxDefaultCpuDispatcher*	m_cpu_dispatcher		= nullptr;

	float m_accumulator	= 0.0f;
	float m_step_size		= 1.0f / 60.0f;

private:
	DISALLOW_COPY_AND_ASSIGN(PhysicsWrapper);
};