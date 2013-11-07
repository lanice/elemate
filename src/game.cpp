#include "game.h"

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
	destroyThread();
	if (m_viewer)
		delete m_viewer;
}

void Game::initialize(){
	m_physics_wrapper.reset(new PhysicsWrapper());

	m_viewer = new osgViewer::Viewer();
	m_viewer->setUpViewInWindow(50, 50, 500, 500);

	m_root = new osg::Geode();
}

void Game::destroyThread(){
	if (m_thread){
		end();
		delete m_thread;
		m_thread = nullptr;
	}
}

void Game::fatalError(string error_message){
	std::cerr << "Game Error occured:" << std::endl;
	std::cerr << error_message << std::endl;
	std::cerr << "Press Enter to close the Application" << std::endl;
	string temp;
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
	destroyThread();
	
	WorldDrawable * world = new WorldDrawable;
	m_root->addDrawable(world);
	m_viewer->setSceneData(m_root.get());
	
	setOsgCamera();
	
	loop();
}

void Game::interrupt(){
	m_interrupted = true;
}

void Game::end(){
	if (isRunning()){
		interrupt();
		if(m_thread)
			m_thread->join();
	}
}

void Game::loop(){
	m_interrupted = false;
	while (!m_viewer->done() && isRunning())
	{
		m_viewer->frame();
		m_physics_wrapper->step(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	m_viewer->setDone(true);
}

bool Game::isRunning()const{
	return !m_interrupted;
}

std::shared_ptr<PhysicsWrapper> Game::physicsWrapper(){
	return m_physics_wrapper;
}