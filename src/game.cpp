#include "game.h"

// Own Classes
#include "physicswrapper.h"
#include "worlddrawable.h"
#include "terraingenerator.h"
#include "godmanipulator.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"

// OSG Classes
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osgTerrain/Terrain>
// #include <osgGA/FirstPersonManipulator>

// PhysX Classes
#include "PxPhysicsAPI.h"

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

	m_cyclic_time = new CyclicTime(0.0L,5.0L);

	m_viewer = new osgViewer::Viewer();
	m_viewer->setUpViewInWindow(50, 50, 500, 500);

	m_root = new osg::Geode();
}

void Game::start(bool spawn_new_thread){
/*	WorldDrawable * world = new WorldDrawable;
	m_root->addDrawable(world);
	m_viewer->setSceneData(m_root.get());*/
    TerrainGenerator * terrainGen = new TerrainGenerator;
    osg::ref_ptr<osgTerrain::Terrain> terrain = terrainGen->getTerrain();
    delete terrainGen;

    m_viewer->setSceneData(terrain);
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
	auto aSphereActor = PxCreateDynamic(*m_physics_wrapper->physics(), physx::PxTransform(physx::PxVec3(0, 1, 0)), physx::PxSphereGeometry(1.0F), *m_physics_wrapper->material("default"), 1.0F);
	aSphereActor->setLinearVelocity(physx::PxVec3(0, 1, 0));
	m_physics_wrapper->scene()->addActor(*aSphereActor);

	//Creates a plane
	physx::PxRigidStatic* plane = PxCreatePlane(*(m_physics_wrapper->physics()), physx::PxPlane(physx::PxVec3(0, 1, 0), 0), *m_physics_wrapper->material("default"));
	m_physics_wrapper->scene()->addActor(*plane);

	auto now = m_cyclic_time->getf();
	auto last_time = m_cyclic_time->getf();
	while (isRunning())
	{
		now = m_cyclic_time->getf(true);
		m_viewer->frame();

		m_physics_wrapper->step(now-last_time);
		last_time = now;
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
	GodManipulator * navigation = new GodManipulator();
	// osgGA::FirstPersonManipulator * navigation = new osgGA::FirstPersonManipulator();
	navigation->setHomePosition(
		osg::Vec3d(0.0, 15.0, 10.0),
		osg::Vec3d(0.0, 0.0, 0.0),
		osg::Vec3d(0.0, 0.0, 1.0));
	navigation->home(0.0);
	m_viewer->setCameraManipulator(navigation);
}

std::shared_ptr<PhysicsWrapper> Game::physicsWrapper() const{
	return m_physics_wrapper;
}