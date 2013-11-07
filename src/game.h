#pragma once

//Own Classes
#include "physicswrapper.h"

// Standard Libs
#include <string>
#include <iostream>
#include <thread>
#include <memory>

using std::string;


#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
	TypeName(const TypeName&);				\
	void operator=(const TypeName&);


/** The Game Class that invokes a game loop and initializes PhysX.
 *	To receive the initialized physics, call getPhysicsWrapper(). See for its usage the documentation of PhysicsWrapper class.
 *  Typical Workflow is instantiating the Game class, call start() and let it run until the game ends, then call end().
 */
class Game{
public:
	explicit Game();	///< Explicit Constructor because Copying and Assignments are disabled
	~Game();

	void start();			///< Starts the Game Loop in a new thread until interrupt() is called
	void interrupt();		///< If a Game loop is running, this function ends it gently
	bool isRunning() const;	///< True if the game loop is running
	void end();				///< Calls interrupt() and waits for finishing ending the loop

	std::shared_ptr<PhysicsWrapper> physicsWrapper();

protected:
	void initialize();
	void fatalError(string error_message); ///< Prints an error message and end the application after pressing enter
	void destroyThread();				   ///< Deletes the thread pointer. If it is currently running, stop it, wait for finishing and delete it afterwards.
	void loop();						   ///< This is where the magic happens. Currently: Physics calculation.

	std::shared_ptr<PhysicsWrapper>	m_physics_wrapper;
	bool							m_interrupted;
	std::thread*					m_thread;

private:
	DISALLOW_COPY_AND_ASSIGN(Game);
};