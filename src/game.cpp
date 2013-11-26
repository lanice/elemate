#include "game.h"

// Own Classes
#include "physicswrapper.h"
#include "objectscontainer.h"
#include "terraingenerator.h"
#include "godmanipulator.h"

// Classes from CGS chair
#include "HPICGS/CyclicTime.h"

// OSG Classes
#include <osgViewer/Viewer>
#include <osgTerrain/Terrain>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>

// PhysX Classes
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

osg::Program * initShader()
{
    osg::ref_ptr<osg::Shader> phongVertex =
        osgDB::readShaderFile("data/phong.vert");
    osg::ref_ptr<osg::Shader> phongFragment =
        osgDB::readShaderFile("data/phong.frag");
    osg::ref_ptr<osg::Shader> phongLightningFragment =
        osgDB::readShaderFile("data/phongLighting.frag");

    osg::Program * phongLightning = new osg::Program();
    phongLightning->addShader(phongVertex.get());
    phongLightning->addShader(phongFragment.get());
    phongLightning->addShader(phongLightningFragment.get());
    return phongLightning;
}

void Game::initialize(osgViewer::Viewer* viewer){
    m_physics_wrapper.reset(new PhysicsWrapper());
    m_objects_container.reset(new ObjectsContainer(m_physics_wrapper));

	m_viewer = viewer;
	
    // use modern OpenGL
    osg::State * graphicsState = m_viewer->getCamera()->getGraphicsContext()->getState();
    graphicsState->setUseModelViewAndProjectionUniforms(true);
    graphicsState->setUseVertexAttributeAliasing(true);

    m_root = new osg::Group();

    osg::Program * phong(initShader());

    m_root->getOrCreateStateSet()->setAttributeAndModes(phong);
}

void Game::start(){
	if (isRunning())
        return;

    m_objects_container->makeStandardBall(m_root, physx::PxVec3( 1, 3, 0), 0.2F, physx::PxVec3(-2, 4, 0), physx::PxVec3(6, 13, 1));
    m_objects_container->makeStandardBall(m_root, physx::PxVec3(-1, 3, 0), 0.2F, physx::PxVec3(2, 4, 0), physx::PxVec3(0, 0, 0));
    m_objects_container->makeStandardBall(m_root, physx::PxVec3(0, 3, 0), 0.2F, physx::PxVec3(0, 0, 0), physx::PxVec3(0, 50, 0));

	
    // create terrain
    TerrainGenerator * terrainGen = new TerrainGenerator();
    terrainGen->setExtentsInWorld(100, 100);
    terrainGen->applySamplesPerWorldCoord(0.5);
    terrainGen->setTilesPerAxis(1, 1);
    terrainGen->setMaxHeight(1.0f);
    m_terrain = std::shared_ptr<ElemateHeightFieldTerrain>(terrainGen->generate());
    delete terrainGen;

    // OSG Object
    m_root->addChild(m_terrain->osgTransformedTerrain());
    // PhysX Object
    for (const auto & actor : m_terrain->pxActorMap()){
        m_physics_wrapper->scene()->addActor(*actor.second);
    }

    // setSceneData also creates the terrain geometry, so we have to pass the geometry to physx after this line
    m_viewer->setSceneData(m_root.get());
    setOsgCamera();

    m_physics_wrapper->startSimulation();
	m_interrupted = false;

	loop();
}

void Game::loop(){
    static int doIt = 100;
	while (isRunning())
    {
        if (--doIt == 0)
            m_objects_container->makeStandardBall(m_root, physx::PxVec3(0, 3, 0), 0.2F, physx::PxVec3(0, 0, 0), physx::PxVec3(0, 50, 0));

        m_viewer->frame();
        m_physics_wrapper->step();
        m_objects_container->updateAllObjects();

        osg::Vec3d eyed, upd, centerd;
        m_navigation->getTransformation(eyed, centerd, upd);
        osg::Vec3 eye(eyed);
        m_root->getOrCreateStateSet()->getOrCreateUniform("cameraposition", osg::Uniform::FLOAT_VEC3)->set(eye);
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
    m_navigation = new GodManipulator();
    m_navigation->setHomePosition(
		osg::Vec3d(0.0, 10.0, 12.0),
		osg::Vec3d(0.0, 2.0, 0.0),
		osg::Vec3d(0.0, 1.0, 0.0));
	m_navigation->home(0.0);
	m_viewer->setCameraManipulator(m_navigation);
}