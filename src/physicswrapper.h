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


protected:
	void initialize();
	void shutdown();
	void fatalError(string error_message);

	physx::PxFoundation*	m_foundation = nullptr;
	physx::PxPhysics*		m_physics = nullptr;

private:
	DISALLOW_COPY_AND_ASSIGN(PhysicsWrapper);
};