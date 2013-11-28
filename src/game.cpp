#include "game.h"

// Own Classes
#include "world.h"
#include "physicswrapper.h"
#include "objectscontainer.h"
#include "godnavigation.h"
#include "godmanipulator.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"

// OSG Classes
#include <osgViewer/Viewer>


Game::Game() : Game(nullptr)
{}

Game::Game(osgViewer::Viewer* viewer) :
m_interrupted(true),
m_viewer(viewer),
m_world(std::make_shared<World>())
{
	// use modern OpenGL
    osg::State * graphicsState = m_viewer->getCamera()->getGraphicsContext()->getState();
    graphicsState->setUseModelViewAndProjectionUniforms(true);
    graphicsState->setUseVertexAttributeAliasing(true);

	m_viewer->setSceneData(m_world->root());
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
    m_viewer->addEventHandler(eventHandler);
    
    setOsgCamera();

    m_world->setNavigation(m_navigation.get());
    m_world->initShader();
    m_world->setUniforms();

    m_world->physics_wrapper->startSimulation();
    m_interrupted = false;

	loop();
}

void Game::loop(){
	while (isRunning())
    {
        m_world->setUniforms();
        m_viewer->frame();
        m_world->physics_wrapper->step();
        m_world->objects_container->updateAllObjects();
    }
	m_interrupted = true;
    m_world->physics_wrapper->stopSimulation();
}

bool Game::isRunning()const{
	return !(m_viewer->done() || m_interrupted);
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
	m_viewer->setCameraManipulator(m_navigation);
}
