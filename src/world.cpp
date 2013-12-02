#include "world.h"

#include "PxPhysicsAPI.h"

#include <osgViewer/Viewer>
#include <osgTerrain/Terrain>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>

#include "physicswrapper.h"
#include "objectscontainer.h"
#include "godnavigation.h"
#include "terraingenerator.h"


World::World()
: physics_wrapper(new PhysicsWrapper())
, objects_container(new ObjectsContainer(physics_wrapper))
, m_root(new osg::Group())
, m_particleGroup(new osg::Group())
{
    m_root->setName("root node");
    m_particleGroup->setName("particle root node");
    m_root->addChild(m_particleGroup.get());

    // Gen Terrain
    TerrainGenerator * terrainGen = new TerrainGenerator();
    terrainGen->setExtentsInWorld(150, 120);
    terrainGen->applySamplesPerWorldCoord(0.2);
    terrainGen->setTilesPerAxis(1, 1);
    terrainGen->setMaxHeight(1.0f);
    //terrainGen->setBiomeSize(5.f); // not supported for now
    terrain = std::shared_ptr<ElemateHeightFieldTerrain>(terrainGen->generate());
    delete terrainGen;

    // OSG Object
    m_root->addChild(terrain->osgTransformedTerrain());

    // PhysX Object
    for (const auto & actor : terrain->pxActorMap()){
        physics_wrapper->scene()->addActor(*actor.second);
    }
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
    objects_container->makeStandardBall(m_particleGroup, physx::PxVec3(1, 3, 0), 0.2F, physx::PxVec3(-2, 4, 0), physx::PxVec3(6, 13, 1));
}

void World::setNavigation(GodNavigation * navigation)
{
    m_navigation = navigation;
}

void World::initShader()
{
    osg::ref_ptr<osg::Shader> terrainVertex =
        osgDB::readShaderFile("shader/terrain.vert");
    std::cout << terrainVertex->getFileName() << std::endl;
    osg::ref_ptr<osg::Shader> terrainGeo = new osg::Shader(osg::Shader::Type::GEOMETRY);
    terrainGeo->setFileName("shader/terrain.geo");
    terrainGeo->loadShaderSourceFromFile(terrainGeo->getFileName());
    osg::ref_ptr<osg::Shader> terrainFragment =
        osgDB::readShaderFile("shader/terrain.frag");
    osg::ref_ptr<osg::Shader> phongLightningFragment =
        osgDB::readShaderFile("shader/phongLighting.frag");

    bool result = terrainVertex.valid() && terrainGeo.valid() && terrainFragment.valid();
    assert(result);

    osg::ref_ptr<osg::Program> terrainProgram = new osg::Program();
    m_programsByName.emplace("terrain", terrainProgram.get());
    terrainProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    terrainProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    terrainProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    terrainProgram->addShader(terrainVertex);
    terrainProgram->addShader(terrainGeo);
    terrainProgram->addShader(terrainFragment);
    terrainProgram->addShader(phongLightningFragment);


    osg::ref_ptr<osg::StateSet> terrainSS = terrain->osgTerrain()->getOrCreateStateSet();
    terrainSS->setAttributeAndModes(terrainProgram.get());

    osg::ref_ptr<osg::Shader> sphereVertex =
        osgDB::readShaderFile("shader/sphere.vert");
    osg::ref_ptr<osg::Shader> sphereFragment =
        osgDB::readShaderFile("shader/sphere.frag");

    osg::ref_ptr<osg::Program> sphereProgram = new osg::Program();
    m_programsByName.emplace("sphere", sphereProgram.get());
    sphereProgram->addShader(sphereVertex);
    sphereProgram->addShader(sphereFragment);
    sphereProgram->addShader(phongLightningFragment);

    osg::ref_ptr<osg::StateSet> sphereSS = m_particleGroup->getOrCreateStateSet();
    sphereSS->setAttributeAndModes(sphereProgram.get());
}

void World::reloadShader()
{
    if (m_programsByName.empty())
        return initShader();

    // reload all shader for all program from source
    for (auto & pair : m_programsByName)
    {
        for (unsigned i = 0; i < pair.second->getNumShaders(); ++i)
        {
            osg::Shader * shader = pair.second->getShader(i);
            shader->loadShaderSourceFromFile(shader->getFileName());
        }
    }
}

osg::Program * World::programByName(std::string name) const
{
    assert(m_programsByName.find(name) != m_programsByName.end());
    return m_programsByName.at(name).get();
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
}
