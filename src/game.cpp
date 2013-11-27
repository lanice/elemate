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
    //graphicsState->setUseVertexAttributeAliasing(true);

	m_viewer->setSceneData(m_world->root());
}

Game::~Game()
{}


void Game::start(){
	if (isRunning())
        return;

    // Add EventHandler to the Viewer that handles events that don't belong to the navigation
    GodManipulator * eventHandler = new GodManipulator();
    eventHandler->setWorld(m_world);
    m_viewer->addEventHandler(eventHandler);
    
    setOsgCamera();

    m_world->physics_wrapper->startSimulation();
	m_interrupted = false;

	loop();
}

void Game::loop(){
	while (isRunning())
	{
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
    osgGA::CameraManipulator * navigation = new GodNavigation();
	navigation->setHomePosition(
		osg::Vec3d(0.0, 10.0, 12.0),
		osg::Vec3d(0.0, 2.0, 0.0),
		osg::Vec3d(0.0, 1.0, 0.0));
	navigation->home(0.0);
	m_viewer->setCameraManipulator(navigation);
}
