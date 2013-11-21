#include "game.h"

//Own Classes
#include "physicswrapper.h"
#include "worlddrawable.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"

// OSG Classes
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgGA/TrackballManipulator>

// PhysxX Classes
#include "PxPhysicsAPI.h"

// Standard Libs
#include <iostream>
#include <thread>
#include <string> 

Game::Game() : Game(nullptr)
{}

Game::Game(osgViewer::Viewer* viewer) :
m_physics_wrapper(nullptr),
m_interrupted(true),
m_viewer(nullptr),
m_root(nullptr)
{
	initialize(viewer);
}

Game::~Game()
{}

void Game::initialize(osgViewer::Viewer* viewer){
	m_physics_wrapper.reset(new PhysicsWrapper());

	m_viewer = viewer;
	
    // use modern OpenGL
    osg::State * graphicsState = m_viewer->getCamera()->getGraphicsContext()->getState();
    graphicsState->setUseModelViewAndProjectionUniforms(true);
    graphicsState->setUseVertexAttributeAliasing(true);

	m_root = new osg::Group();
}

void Game::start(){
	if (isRunning())
		return;
	
    // Creates a Sphere
    //OSG Object
    m_sphere1.first = new osg::MatrixTransform();
    osg::ref_ptr<osg::Geode> sphere1_geode = new osg::Geode();
    sphere1_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), 0.2)));
    m_sphere1.first->addChild(sphere1_geode);
    m_root->addChild(m_sphere1.first);
    //PhysX Object
    m_sphere1.second = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(physx::PxVec3(1, 3, 0)), physx::PxSphereGeometry(0.2F), *m_physics_wrapper->material("default"), 1.0F);
    m_sphere1.second->setLinearVelocity(physx::PxVec3(-2, 4.0, 0));
    m_physics_wrapper->scene()->addActor(*m_sphere1.second);

    // Creates a Sphere
    //OSG Object
    m_sphere2.first = new osg::MatrixTransform();
    osg::ref_ptr<osg::Geode> sphere2_geode = new osg::Geode();
    sphere2_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 0), 0.2)));
    m_sphere2.first->addChild(sphere2_geode);
    m_root->addChild(m_sphere2.first);
    //PhysX Object
    m_sphere2.second = PxCreateDynamic(PxGetPhysics(), physx::PxTransform(physx::PxVec3(-1, 3, 0)), physx::PxSphereGeometry(0.2F), *m_physics_wrapper->material("default"), 1.0F);
    m_sphere2.second->setLinearVelocity(physx::PxVec3(2, 4.0, 0));
    m_physics_wrapper->scene()->addActor(*m_sphere2.second);

	//Creates a plane
        //OSG Object
    osg::ref_ptr<WorldDrawable> world = new WorldDrawable;
    osg::ref_ptr<osg::Geode> world_geode = new osg::Geode();
    world_geode->addDrawable(world.get());
    m_root->addChild(world_geode.get());
        //PhysXObject
    physx::PxRigidStatic* plane = PxCreatePlane(PxGetPhysics(), physx::PxPlane(physx::PxVec3(0, 1, 0), 0.0F), *m_physics_wrapper->material("default"));
	m_physics_wrapper->scene()->addActor(*plane);
	
	m_viewer->setSceneData(m_root.get());	
	setOsgCamera();

    m_physics_wrapper->startSimulation();
	m_interrupted = false;

	loop();
}

void Game::loop(){
	while (isRunning())
	{
        m_viewer->frame();

        m_physics_wrapper->step();

		m_physics_wrapper->scene()->fetchResults();

        m_sphere1.first->setMatrix(osg::Matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, m_sphere1.second->getGlobalPose().p.x, m_sphere1.second->getGlobalPose().p.y, m_sphere1.second->getGlobalPose().p.z, 1));
        m_sphere2.first->setMatrix(osg::Matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, m_sphere2.second->getGlobalPose().p.x, m_sphere2.second->getGlobalPose().p.y, m_sphere2.second->getGlobalPose().p.z, 1));
	}
	m_interrupted = true;
    m_physics_wrapper->stopSimulation();
}

bool Game::isRunning()const{
	return !(m_viewer->done() || m_interrupted);
}

void Game::end(){
	if (isRunning())
		m_interrupted = true;
}

void Game::setOsgCamera(){
	osgGA::TrackballManipulator * navigation = new osgGA::TrackballManipulator();
	navigation->setHomePosition(
		osg::Vec3d(0.0, 6.0, 12.0),
		osg::Vec3d(0.0, 3.0, 0.0),
		osg::Vec3d(0.0, 1.0, 0.0));
	navigation->home(0.0);
	m_viewer->setCameraManipulator(navigation);
}

std::shared_ptr<PhysicsWrapper> Game::physicsWrapper() const{
	return m_physics_wrapper;
}