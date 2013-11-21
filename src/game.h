#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
	TypeName(const TypeName&);				\
	void operator=(const TypeName&);

#include <osg/GraphicsContext> // ref_ptr
#include <memory> //shared_ptr

//Forward Declarations
class PhysicsWrapper;
namespace std {			class thread; }
namespace osg {			class Group;  
						class MatrixTransform;
}
namespace osgViewer {	class Viewer; }
namespace physx{ class PxRigidDynamic; }

/** The Game Class that invokes a game loop and initializes PhysX.
 *	To receive the initialized physics, call getPhysicsWrapper(). See for its usage the documentation of PhysicsWrapper class.
 *  Typical Workflow is instantiating the Game class, call start() and let it run until the game ends, then call end().
 */
class Game{
public:

	typedef std::pair<osg::MatrixTransform*, physx::PxRigidDynamic*> DrawableAndPhysXObject;

	/** Explicit Constructor because Copying and Assignments are disabled. 	*/
	explicit Game();	
	explicit Game(osgViewer::Viewer* viewer);

	~Game();

	/** Starts the Game Loop until end() is called.  */
	void start();

	/** True if the game loop is running. */
	bool isRunning() const;

	/** Ending the loop. */
	void end();

	/** Returns properly initialized PhysicsWrapper. */
	std::shared_ptr<PhysicsWrapper> physicsWrapper() const;

protected:
	void initialize(osgViewer::Viewer* viewer);

	/** The Game's loop containing drawing and triggering physics is placed right here. */
	void loop();

	/** This is where the magic happens. Currently: Physics calculation. */
	void setOsgCamera();

	std::shared_ptr<PhysicsWrapper>	m_physics_wrapper;
	osgViewer::Viewer*				m_viewer;
	bool							m_interrupted;
	osg::ref_ptr<osg::Group>		m_root;
	
    DrawableAndPhysXObject			m_sphere1;
    DrawableAndPhysXObject			m_sphere2;

private:
	DISALLOW_COPY_AND_ASSIGN(Game);
};