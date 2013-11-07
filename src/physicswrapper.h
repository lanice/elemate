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

/** This Class initializes all basic objects that are necessary to use NVIDIA Physics.
 * At the moment, the properly initialized physics and scene object are accessible via functions.
 * Using this class is simple: Istantiate it and you are able to use the already initialized physics and scene. To make a step in scene-simulation, call step() with the passed time.
 * We still have to decide if we want to wrap the standard functions of PhysX like addActor and athe creation of rigid actors themselves into a separate function.
 * The PxProfilZoneManager is currently disabled, but required code in Constructor, Destructor and Header file is present.
 */
class PhysicsWrapper{
public:
	explicit PhysicsWrapper();	///< Explicit Constructor because Copying and Assignments are disabled.
	~PhysicsWrapper();			

	bool	step(physx::PxReal dt);			///< Proceeds with simulation for amount of given time and returns if enough time has passed to do another step.

	physx::PxPhysics*	physics() const;	///< The returned object is initialized.
	physx::PxScene*		scene() const;		///< The returned object is initialized.

protected:
	static const int	kNumberOfThreads;	///< Default value is 2. Number of threads is required for the CPU Dispatcher of th PhysX library.
	static const float	kDefaultStepSize;	///< The step size reqiured for a simulation step. There is an accumulator which saves past steps until they reach the step size.

	void initializePhysics();								///< Creation of PxFoundation, PxPhysics and Initialization of PxExtensions.
	void initializeScene();									///< Creation of PxDefaultCpuDispatcher and PxScene after customized SceneDescription provided by customizeSceneDescription()
	void customizeSceneDescription(physx::PxSceneDesc&);	///< Specifies special scene description. 
	void shutdown();										///< Releases all members to free their instances. Automatically called in Destructor.
	void fatalError(string error_message);					///< Prints an error message and end the application after pressing enter.

	physx::PxFoundation*				m_foundation;			///< 
	//physx::PxProfileZoneManager*		m_profile_zone_manager; ///< currently disabled.
	physx::PxDefaultCpuDispatcher*		m_cpu_dispatcher;		///< 
	physx::PxPhysics*					m_physics;				///< 
	physx::PxScene*						m_scene;				///< 

	float m_accumulator;
	float m_step_size;

private:
	DISALLOW_COPY_AND_ASSIGN(PhysicsWrapper);
};