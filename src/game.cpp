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
	m_interrupted(true),
	m_physics_wrapper(nullptr),
	m_thread(nullptr),
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
}

void Game::initialize(){
	m_physics_wrapper.reset(new PhysicsWrapper());

	m_viewer = new osgViewer::Viewer();
	m_viewer->setUpViewInWindow(50, 50, 500, 500);

	m_root = new osg::Geode();
}

void Game::fatalError(std::string error_message){
	std::cerr << "Game Error occured:" << std::endl;
	std::cerr << error_message << std::endl;
	std::cerr << "Press Enter to close the Application" << std::endl;
	std::string temp;
	std::getline(std::cin, temp);
	exit(1);
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

void Game::start(bool spawn_new_thread){
	WorldDrawable * world = new WorldDrawable;
	m_root->addDrawable(world);
	m_viewer->setSceneData(m_root.get());
	
	setOsgCamera();
	
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

void Game::end(){
	if (isRunning()){
		m_viewer->setDone(true);
		if(m_thread)
			m_thread->join();
	}
}

void Game::loop(){
	m_interrupted = false;
	while (isRunning())
	{
		m_viewer->frame();
		m_physics_wrapper->step(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	m_viewer->setDone(true);
}

bool Game::isRunning()const{
	return !m_viewer->done();
}

std::shared_ptr<PhysicsWrapper> Game::physicsWrapper(){
	return m_physics_wrapper;
}