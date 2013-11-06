#pragma once

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


private:
	DISALLOW_COPY_AND_ASSIGN(PhysicsWrapper);
};