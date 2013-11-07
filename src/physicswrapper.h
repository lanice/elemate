#pragma once

// Standard Libs
#include <string>
#include <iostream>
#include <memory>

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

	bool								step(physx::PxReal dt);

	physx::PxPhysics*	physics() const;
	physx::PxScene*		scene() const;

protected:
	static const int	kNumberOfThreads;
	static const float	kDefaultStepSize;

	void initializePhysics();
	void initializeScene();
	void customizeSceneDescription(physx::PxSceneDesc&);
	void shutdown();
	void fatalError(string error_message);

	physx::PxFoundation*				m_foundation;
	physx::PxProfileZoneManager*		m_profile_zone_manager;
	physx::PxDefaultCpuDispatcher*		m_cpu_dispatcher;
	physx::PxPhysics*					m_physics;
	physx::PxScene*						m_scene;

	float m_accumulator;
	float m_step_size;

private:
	DISALLOW_COPY_AND_ASSIGN(PhysicsWrapper);
};