#include "game.h"

#include <iostream>
#include <thread>
#include <chrono>

// OSG Classes
#include <osgViewer/Viewer>

// Own Classes
#include "world.h"
#include "physicswrapper.h"
#include "objectscontainer.h"
#include "godnavigation.h"
#include "godmanipulator.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"


Game::Game(osgViewer::Viewer& viewer) :
m_interrupted(true),
m_viewer(viewer),
m_world(std::make_shared<World>()),
m_cyclicTime(new CyclicTime(0.0L, 1.0L))
{
    // create new context traits to configure vsync etc
    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits(*m_viewer.getCamera()->getGraphicsContext()->getTraits());

    traits->windowName = "Elemate";
    traits->vsync = true;
    // traits->useCursor = false;

    // apply new settings viewer
    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    m_viewer.getCamera()->setGraphicsContext(gc.get());

	// use modern OpenGL
    osg::State * graphicsState = m_viewer.getCamera()->getGraphicsContext()->getState();
    graphicsState->setUseModelViewAndProjectionUniforms(true);
    graphicsState->setUseVertexAttributeAliasing(true);

	m_viewer.setSceneData(m_world->root());
}

Game::~Game()
{}

void Game::start(){
	if (isRunning())
        return;

    // Add GodManipulator (event handler) to the Viewer that handles events
    // that don't belong to the navigation but to game content/logic. 
    // It is added to Viewers EventHandlerQueue to receive incoming events.
    GodManipulator * eventHandler = new GodManipulator();
    // The handler gets access to the World to process game content/logic specific events.
    eventHandler->setWorld(m_world);
    m_viewer.addEventHandler(eventHandler);
    
    setOsgCamera();

    m_world->setNavigation(m_navigation.get());
    m_world->reloadShader();
    m_world->setUniforms();

    m_world->physics_wrapper->startSimulation();

	loop();
}

void Game::loop(t_longf delta){
	m_interrupted = false;

	t_longf nextTime = m_cyclicTime->getNonModf(true);
	t_longf maxTimeDiff = 0.5L;
	int skippedFrames = 1;
	int maxSkippedFrames = 5;

	while (isRunning())
    {
		// get current time
		t_longf currTime = m_cyclicTime->getNonModf(true);

		// are we too far far behind? then do drawing step now.
		if ((currTime - nextTime) > maxTimeDiff)
			nextTime = currTime;

		if (currTime >= nextTime)
		{
			nextTime += delta;

			// update physic
            if (m_world->physics_wrapper->step())
                // physx: each simulate() call must be followed by fetchResults()
                m_world->objects_container->updateAllObjects();

	        // update and draw objects if we have time remaining or already too many frames skipped.
	        if ((currTime < nextTime) || (skippedFrames > maxSkippedFrames))
	        {
		        m_world->setUniforms();
		        m_viewer.frame();
		        skippedFrames = 1;
	        } else {
	        	++skippedFrames;
	        }
	    } else {
	    	t_longf sleepTime = nextTime - currTime;

	    	if (sleepTime > 0)
	    		std::this_thread::sleep_for(std::chrono::milliseconds(int(sleepTime * 1000)));
	    }
    }

	m_interrupted = true;
    m_world->physics_wrapper->stopSimulation();
}

bool Game::isRunning()const{
	return !(m_viewer.done() || m_interrupted);
}

void Game::end(){
	if (isRunning())
		m_interrupted = true;
}

void Game::setOsgCamera(){
    m_navigation = new GodNavigation();
    m_navigation->setHomePosition(
		osg::Vec3d(0.0, 10.0, 12.0),
		osg::Vec3d(0.0, 2.0, 0.0),
		osg::Vec3d(0.0, 1.0, 0.0));
	m_navigation->home(0.0);
	m_viewer.setCameraManipulator(m_navigation);
}
