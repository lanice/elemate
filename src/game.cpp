#include "game.h"

//Own Classes
#include "physicswrapper.h"
#include "objectscontainer.h"
#include "terraingenerator.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"

// OSG Classes
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgTerrain/Terrain>
#include <osg/MatrixTransform>

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
m_objects_container(nullptr),
m_interrupted(true),
m_viewer(nullptr),
m_root(nullptr),
m_terrain(nullptr)
{
	initialize(viewer);
}

Game::~Game()
{}

void Game::initialize(osgViewer::Viewer* viewer){
    m_physics_wrapper.reset(new PhysicsWrapper());
    m_objects_container.reset(new ObjectsContainer(m_physics_wrapper));

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

    // Set light source
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum(1);
    light->setPosition(osg::Vec4(0, 7, 0, 1.0f));

    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->setLight(light.get());

    m_root->addChild(lightSource.get());
    lightSource->setStateSetModes(*m_root->getOrCreateStateSet(),
        osg::StateAttribute::ON);

    m_objects_container->makeStandardBall(m_root, physx::PxVec3( 1, 3, 0), 0.2F, physx::PxVec3(-2, 4, 0), physx::PxVec3(6, 13, 1));
    m_objects_container->makeStandardBall(m_root, physx::PxVec3(-1, 3, 0), 0.2F, physx::PxVec3(2, 4, 0), physx::PxVec3(0, 0, 0));
    m_objects_container->makeStandardBall(m_root, physx::PxVec3(0, 3, 0), 0.2F, physx::PxVec3(0, 0, 0), physx::PxVec3(0, 50, 0));
    //m_objects_container->makePlane(m_root);

	
    // create terrain
    TerrainGenerator * terrainGen = new TerrainGenerator();
    m_terrain = std::shared_ptr<ElemateHeightFieldTerrain>(terrainGen->generate());
    delete terrainGen;

    // OSG Object
    m_root->addChild(m_terrain->osgTransformedTerrain());
    // PhysX Object
    m_physics_wrapper->scene()->addActor(*(m_terrain->pxActor(osgTerrain::TileID(0, 0, 0))));

    // setSceneData also creates the terrain geometry, so we have to pass the geometry to physx after this line
    m_viewer->setSceneData(m_root.get());
    setOsgCamera();

    m_physics_wrapper->startSimulation();
	m_interrupted = false;

	loop();
}

void Game::loop(){
    t_longf newBallRounds = 10.0F;
	while (isRunning())
	{
        m_viewer->frame();
        m_physics_wrapper->step();
        m_objects_container->updateAllObjects();
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
		osg::Vec3d(0.0, 10.0, 12.0),
		osg::Vec3d(0.0, 2.0, 0.0),
		osg::Vec3d(0.0, 1.0, 0.0));
	navigation->home(0.0);
	m_viewer->setCameraManipulator(navigation);
}