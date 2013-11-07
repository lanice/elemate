#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
	TypeName(const TypeName&);				\
	void operator=(const TypeName&);

#include <osg/GraphicsContext> // ref_ptr
#include <memory> //shared_ptr

//Forward Declarations
class PhysicsWrapper;
namespace std {			class thread; }
namespace osg {			class Geode;  }
namespace osgViewer {	class Viewer; }

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

	/** True if the game loop is running. */
	bool isRunning() const;

	/** Calls interrupt() and waits for finishing ending the loop. */
	void end();

	/** Returns properly initialized PhysicsWrapper. */
	std::shared_ptr<PhysicsWrapper> physicsWrapper();

protected:
	void initialize();

	/** Prints an error message and end the application after pressing enter. */
	void fatalError(std::string error_message);

	/** The Game's loop containing drawing and triggering physics is placed right here. */
	void loop();

	/** This is where the magic happens. Currently: Physics calculation. */
	void setOsgCamera();

	std::shared_ptr<PhysicsWrapper>	m_physics_wrapper;
	bool							m_interrupted;
	std::thread*					m_thread;
	osgViewer::Viewer*				m_viewer;
	osg::ref_ptr<osg::Geode>		m_root;


private:
	DISALLOW_COPY_AND_ASSIGN(Game);
};