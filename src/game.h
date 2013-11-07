#pragma once

//Own Classes
#include "physicswrapper.h"
#include "worlddrawable.h"

//OSG Classes
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osg/GraphicsContext>
#include <osgGA/TrackballManipulator>

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

	/** Explicit Constructor because Copying and Assignments are disabled. 	*/
	explicit Game();	
	
	~Game();

	/** 
	  * Starts the Game Loop in a new thread until interrupt() is called. 	
	  * @param spawn_new_thread If set true, the game loop is executed in a new thread and start() is not a blocking call.
	  */
	void start(bool spawn_new_thread);

	/** If a Game loop is running, this function ends it gently. */
	void interrupt();

	/** True if the game loop is running. */
	bool isRunning() const;

	/** Calls interrupt() and waits for finishing ending the loop. */
	void end();

	/** Returns properly initialized PhysicsWrapper. */
	std::shared_ptr<PhysicsWrapper> physicsWrapper();

protected:
	void initialize();

	/** Prints an error message and end the application after pressing enter. */
	void fatalError(string error_message);

	/** Deletes the thread pointer. If it is currently running, stop it, wait for finishing and delete it afterwards. */
	void destroyThread();

	/** This is where the magic happens. Currently: Physics calculation. */
	void loop();

	void setOsgCamera(); ///< Doesn't really belong here

	std::shared_ptr<PhysicsWrapper>	m_physics_wrapper;
	bool							m_interrupted;
	std::thread*					m_thread;
	osgViewer::Viewer*				m_viewer;		///< Doesn't really belong here
	osg::ref_ptr<osg::Geode>		m_root;			///< Doesn't really belong here


private:
	DISALLOW_COPY_AND_ASSIGN(Game);
};