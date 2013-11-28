#include "world.h"

#include "physicswrapper.h"
#include "objectscontainer.h"
#include "godnavigation.h"
#include "terraingenerator.h"

#include "PxPhysicsAPI.h"

#include <osgViewer/Viewer>
#include <osgTerrain/Terrain>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>


World::World()
{
    physics_wrapper.reset(new PhysicsWrapper());
    objects_container.reset(new ObjectsContainer(physics_wrapper));

    m_root = new osg::Group();
    m_root->setName("root node");


    // Gen Terrain
    TerrainGenerator * terrainGen = new TerrainGenerator();
    terrainGen->setExtentsInWorld(100, 100);
    terrainGen->applySamplesPerWorldCoord(0.5);
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
    objects_container->makeStandardBall(m_root, physx::PxVec3( 1, 3, 0), 0.2F, physx::PxVec3(-2, 4, 0), physx::PxVec3(6, 13, 1));
}

void World::setNavigation(GodNavigation * navigation)
{
    m_navigation = navigation;
}

void World::initShader()
{
    assert(m_programsByName.empty());   // until we implement cleanup/reload shaders

    osg::ref_ptr<osg::Shader> terrainVertex =
        osgDB::readShaderFile("shader/terrain.vert");
    osg::ref_ptr<osg::Shader> terrainFragment =
        osgDB::readShaderFile("shader/terrain.frag");
    osg::ref_ptr<osg::Shader> phongLightningFragment =
        osgDB::readShaderFile("shader/phongLighting.frag");

    osg::ref_ptr<osg::Program> terrainProgram = new osg::Program();
    terrainProgram->addShader(terrainVertex.get());
    terrainProgram->addShader(terrainFragment.get());
    terrainProgram->addShader(phongLightningFragment.get());
    m_programsByName.emplace("terrain", terrainProgram.get());


    osg::ref_ptr<osg::StateSet> terrainSS = terrain->osgTerrain()->getOrCreateStateSet();
    terrainSS->setAttributeAndModes(terrainProgram);
}

osg::Program * World::programByName(std::string name) const
{
    assert(m_programsByName.find(name) != m_programsByName.end());
    return m_programsByName.at(name);
}

void World::setUniforms()
{
    assert(!m_programsByName.empty()); // we don't want to set uniforms when we do not have shaders
    assert(m_navigation.valid());

    osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);
    osg::Vec3 eye(eyed);
    m_root->getOrCreateStateSet()->getOrCreateUniform("cameraposition", osg::Uniform::FLOAT_VEC3)->set(eye);

    osg::ref_ptr<osg::StateSet> terrainSS = terrain->osgTerrain()->getOrCreateStateSet();
    // texture unit 0 should be color layer 0 in all tiles
    /*terrainSS->getOrCreateUniform("terrainType", osg::Uniform::Type::UNSIGNED_INT_SAMPLER_2D)->set(0);
    terrainSS->getOrCreateUniform("tileSize", osg::Uniform::Type::FLOAT_VEC3)->set(osg::Vec3(
        terrain->settings().tileSizeX(),
        terrain->settings().maxHeight,
        terrain->settings().tileSizeZ()));*/

    //osg::StateSet::TextureAttributeList texs = m_terrain->osgTerrain()->getTile(osgTerrain::TileID(0,0,0))->getOrCreateStateSet()->getTextureAttributeList();
    //for (const osg::StateSet::AttributeList & tex : texs)
    //{
    //    for (const std::pair<osg::StateAttribute::TypeMemberPair, std::pair<osg::ref_ptr<osg::StateAttribute>, osg::StateAttribute::OverrideValue>> & x : tex)
    //    {
    //        osg::Texture * me = dynamic_cast<osg::Texture*>(x.second.first.get());
    //    }
    //}
}
