#include "world.h"

#include "physicswrapper.h"
#include "objectscontainer.h"
#include "terraingenerator.h"
#include "soundmanager.h"

#include "PxPhysicsAPI.h"

#include <osgViewer/Viewer>
#include <osgTerrain/Terrain>
#include <osg/MatrixTransform>


World::World()
{
	physics_wrapper.reset(new PhysicsWrapper());
	objects_container.reset(new ObjectsContainer(physics_wrapper));
	soundManager.reset(new SoundManager());

    m_root = new osg::Group();

	//Config SoundManager
	soundManager->createNewChannel("data/sounds/birds.mp3", true, false);
	soundManager->createNewChannel("data/sounds/spring.mp3", true, true, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });
	soundManager->createNewChannel("data/sounds/bomb.wav", false, true, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f });


    // Gen Terrain
    TerrainGenerator * terrainGen = new TerrainGenerator();
    terrainGen->setExtentsInWorld(10, 10);
    terrainGen->setSamplesPerWorldXCoord(0.5);
    terrainGen->setSamplesPerWorldZCoord(0.5);
    terrainGen->setTilesPerAxis(1, 1);
    terrainGen->setMaxHeight(1.0f);
    terrain = std::shared_ptr<ElemateHeightFieldTerrain>(terrainGen->generate());
    delete terrainGen;

    // OSG Object
    m_root->addChild(terrain->osgTransformedTerrain());

    // PhysX Object
    for (const auto & actor : terrain->pxActorMap()){
        physics_wrapper->scene()->addActor(*actor.second);
    }


    // Set Light Source
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum(1);
    light->setPosition(osg::Vec4(-10, 4, 5, 1.0f));

    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->setLight(light.get());

    m_root->addChild(lightSource.get());
    lightSource->setStateSetModes(*m_root->getOrCreateStateSet(), osg::StateAttribute::ON);
}


World::~World()
{
}


osg::Group* World::root()
{
    return m_root.get();
}

void World::makeStandardBall()
{
	// prototype: hard-coded physx values etc.
	objects_container->makeStandardBall(m_root, physx::PxVec3(1, 3, 0), 0.2F, physx::PxVec3(-2, 4, 0), physx::PxVec3(6, 13, 1));
}

void World::moveSoundLeft(int channelId){
	soundManager->moveSound(channelId, { -0.2f, 0.f, 0.f });
}

void World::moveSoundRight(int channelId){
	soundManager->moveSound(channelId, { 0.2f, 0.f, 0.f });
}

void World::moveSoundForw(int channelId){
	soundManager->moveSound(channelId, { 0.f, 0.2f, 0.f });
}

void World::moveSoundBackw(int channelId){
	soundManager->moveSound(channelId, { 0.f, -0.2f, 0.f });
}

void World::moveSoundUp(int channelId){
	soundManager->moveSound(channelId, { 0.f, 0.f, 0.2f });
}

void World::moveSoundDown(int channelId){
	soundManager->moveSound(channelId, { 0.f, 0.f, -0.2f });
}

void World::doBomb(int channelId){
	soundManager->play(channelId);
}