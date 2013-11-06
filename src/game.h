#pragma once

//Own Classes
#include "physicswrapper.h"

// Standard Libs
#include <string>
#include <iostream>
#include <thread>

using std::string;

// NVIDIA PhysX Libs
#include "PxPhysicsAPI.h"


class Game{
public:
	explicit Game();
	~Game();

	void start();
	void interrupt();
	bool isRunning();
	void end();

protected:
	void initialize();
	void fatalError(string error_message);
	void loop();

	PhysicsWrapper*	m_physics_wrapper;
	bool			m_interrupted;
	std::thread*	m_thread;

private:
	DISALLOW_COPY_AND_ASSIGN(Game);
};