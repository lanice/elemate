#include "game.h"

//Own Classes
#include "physicswrapper.h"
//#include "worlddrawable.h"
#include "terraingenerator.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"

// OSG Classes
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osgGA/TrackballManipulator>
#include <osgTerrain/Terrain>

// PhysxX Classes
#include "PxPhysicsAPI.h"
#include <geometry/PxTriangleMeshGeometry.h>

// Standard Libs
#include <iostream>
#include <thread>
#include <string> 

Game::Game() : Game(nullptr)
{}

Game::Game(osgViewer::Viewer* viewer) :
m_physics_wrapper(nullptr),
m_interrupted(true),
m_cyclic_time(nullptr),
m_viewer(nullptr),
m_root(nullptr)
{
	initialize(viewer);
}

Game::~Game(){
	if (m_cyclic_time)
		delete m_cyclic_time;
}

void Game::initialize(osgViewer::Viewer* viewer){
	m_physics_wrapper.reset(new PhysicsWrapper());

	m_cyclic_time = new CyclicTime(0.0L,5.0L);
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

    // create terrain
    TerrainGenerator * terrainGen = new TerrainGenerator();
    osg::ref_ptr<osgTerrain::Terrain> terrain = terrainGen->getTerrain();
    delete terrainGen;
    m_root->addChild(terrain.get());
	
	// Creates a Sphere
	m_sphere.first = new osg::MatrixTransform();
	osg::ref_ptr<osg::Geode> sphere_geode = new osg::Geode();
	sphere_geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0, 0, 5), 1)));
	m_sphere.first->addChild(sphere_geode);
	m_root->addChild(m_sphere.first);
	m_sphere.second = PxCreateDynamic(*m_physics_wrapper->physics(), physx::PxTransform(physx::PxVec3(0, 0, 5)), physx::PxSphereGeometry(1.0F), *m_physics_wrapper->material("default"), 1.0F);
	m_sphere.second->setLinearVelocity(physx::PxVec3(0, 1, 0));
	m_physics_wrapper->scene()->addActor(*m_sphere.second);

    // setSceneData also creates the terrain geometry, so we have to pass the geometry to physx after this line
    m_viewer->setSceneData(m_root.get());
    setOsgCamera();

    //Creates a plane
    physx::PxTriangleMeshGeometry * pxTerrainGeo = TerrainGenerator::pxTerrainGeometry(terrain.get());
    physx::PxRigidStatic * pxTerrain = PxCreateStatic(PxGetPhysics(),
        physx::PxTransform(),
        *pxTerrainGeo,
        *m_physics_wrapper->material("default"));
    m_physics_wrapper->scene()->addActor(*pxTerrain);
    

	//physx::PxRigidStatic* plane = PxCreatePlane(*(m_physics_wrapper->physics()), physx::PxPlane(physx::PxVec3(0, 1, 0), 0), *m_physics_wrapper->material("default"));
	//m_physics_wrapper->scene()->addActor(*plane);

	m_cyclic_time->start();
	m_interrupted = false;

	loop();
}

void Game::loop(){
	t_longf now = m_cyclic_time->getf();
	t_longf last_time = m_cyclic_time->getf();
	while (isRunning())
	{
		now = m_cyclic_time->getf(true);
		m_viewer->frame();

		m_physics_wrapper->step(now-last_time);
		last_time = now;

		m_physics_wrapper->scene()->fetchResults();
		/*std::cout	<< "("	<< m_sphere.second->getGlobalPose().p.x << ", " 
							<< m_sphere.second->getGlobalPose().p.y << ", " 
							<< m_sphere.second->getGlobalPose().p.z << ")" << std::endl;*/
		m_sphere.first->setMatrix(osg::Matrix(1,0,0,0, 0,1,0,0, 0,0,1,0, m_sphere.second->getGlobalPose().p.x, m_sphere.second->getGlobalPose().p.y, m_sphere.second->getGlobalPose().p.z, 1 ));
	}
	m_interrupted = true;
	m_cyclic_time->stop();
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
		osg::Vec3d(0.0, 15.0, 10.0),
		osg::Vec3d(0.0, 0.0, 0.0),
		osg::Vec3d(0.0, 0.0, 1.0));
	navigation->home(0.0);
	m_viewer->setCameraManipulator(navigation);
}

std::shared_ptr<PhysicsWrapper> Game::physicsWrapper() const{
	return m_physics_wrapper;
}