#include "game.h"

//Own Classes
#include "physicswrapper.h"
#include "worlddrawable.h"

//Classes from CGS chair
#include "HPICGS/CyclicTime.h"

//OSG Classes
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osgGA/TrackballManipulator>

// Standard Libs
#include <iostream>
#include <thread>
#include <string> 

Game::Game() :
	m_physics_wrapper(nullptr),
	m_thread(nullptr),
	m_cyclic_time(nullptr),
	m_viewer(nullptr),
	m_root(nullptr)
{
	initialize();
}

Game::~Game(){
	if (m_thread){
		end();
		delete m_thread;
		m_thread = nullptr;
	}
	if (m_viewer)
		delete m_viewer;
	if (m_cyclic_time)
		delete m_cyclic_time;
}

void Game::initialize(){
	m_physics_wrapper.reset(new PhysicsWrapper());

	m_cyclic_time = new CyclicTime();

	m_viewer = new osgViewer::Viewer();
	m_viewer->setUpViewInWindow(50, 50, 500, 500);

	m_root = new osg::Geode();
}

void Game::start(bool spawn_new_thread){
	WorldDrawable * world = new WorldDrawable;
	m_root->addDrawable(world);
	m_viewer->setSceneData(m_root.get());	
	setOsgCamera();

	m_cyclic_time->start();

	if (spawn_new_thread){
		if (m_thread){
			end();
			delete m_thread;
			m_thread = nullptr;
		}
		m_thread = new std::thread(&Game::loop, this);
	}
	else {
		loop();
	}
}

void Game::loop(){
	while (isRunning())
	{
		m_viewer->frame();
		m_cyclic_time->update();
		m_physics_wrapper->step(m_cyclic_time->getf());
	}
	m_viewer->setDone(true);
	m_cyclic_time->stop();
}

bool Game::isRunning()const{
	return !m_viewer->done();
}

void Game::end(){
	if (isRunning()){
		m_viewer->setDone(true);
		if(m_thread)
			m_thread->join();
	}
}

void Game::setOsgCamera(){
	osgGA::TrackballManipulator * navigation = new osgGA::TrackballManipulator();
	navigation->setHomePosition(
		osg::Vec3d(0.0, 3.0, 2.0),
		osg::Vec3d(0.0, 0.0, 0.0),
		osg::Vec3d(0.0, 1.0, 0.0));
	navigation->home(0.0);
	m_viewer->setCameraManipulator(navigation);
}

std::shared_ptr<PhysicsWrapper> Game::physicsWrapper() const{
	return m_physics_wrapper;
}