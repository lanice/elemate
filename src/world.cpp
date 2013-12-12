#include "world.h"

#include "PxPhysicsAPI.h"

#include <osgViewer/Viewer>
#include <osgTerrain/Terrain>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>

#include "physicswrapper.h"
#include "objectscontainer.h"
#include "godnavigation.h"
#include "terrain/terraingenerator.h"
#include "soundmanager.h"


World::World()
: physics_wrapper(new PhysicsWrapper())
, objects_container(new ObjectsContainer(physics_wrapper))
, soundManager(new SoundManager())
, m_root(new osg::Group())
, m_particleGroup(new osg::Group())
{
    m_root->setName("root node");
    m_particleGroup->setName("particle root node");
    m_root->addChild(m_particleGroup.get());

    // Create two non-3D channels (paino and rain)
    //initialise as paused
    soundManager->createNewChannel("data/sounds/rain.mp3", true, false, true);
    soundManager->createNewChannel("data/sounds/piano.mp3", true, false, true);
    //set volume (make quieter)
    soundManager->setVolume(0, 0.2f);
    soundManager->setVolume(1, 0.5f);
    //end pause
    soundManager->setPaused(0, false);
    soundManager->setPaused(1, false);

    // Gen Terrain
    TerrainGenerator * terrainGen = new TerrainGenerator();
    terrainGen->setExtentsInWorld(150, 200);
    terrainGen->applySamplesPerWorldCoord(2.f);
    terrainGen->setTilesPerAxis(1, 1);
    terrainGen->setMaxHeight(20.0f);
    terrainGen->setMaxBasicHeightVariance(0.05f);
    terrain = std::shared_ptr<ElemateHeightFieldTerrain>(terrainGen->generate());
    delete terrainGen;

    // OSG Object
    m_root->addChild(terrain->osgTransformedTerrain());

    // PhysX Object
    for (const auto & actor : terrain->pxActorMap()){
        physics_wrapper->scene()->addActor(*actor.second);
    }

    setUpCameraDebugger();

    setUpLighting();
}


World::~World()
{
}

void World::setGraphicsContext(osg::GraphicsContext * context)
{
    m_graphicContext = context;
    physics_wrapper->setOsgGraphicsContext(context);
}

void World::setUpCameraDebugger()
{
    m_cameraDebugger = new osg::MatrixTransform();

    osg::ref_ptr<osg::Geode> sphere_geode = new osg::Geode();
    sphere_geode->addDrawable(new osg::ShapeDrawable(new osg::Cone(osg::Vec3(0, 0, 0), 0.2, 1.0)));
    m_cameraDebugger->addChild(sphere_geode);
    m_particleGroup->addChild(m_cameraDebugger.get());
}

void World::setUpLighting()
{
    osg::Vec4 lightambientglobal(0, 0, 0, 0);
    osg::Vec3 lightdir1(0.0, 6.5, 7.5);
    osg::Vec3 lightdir2(0.0, -8.0, 7.5);

    // some kind of sunlight..
    osg::Matrixf light1(0.0, 0.0, 0.0, 1.0,        //ambient
        0.2, 0.2, 0.2, 1.0,        //diffuse
        0.7, 0.7, 0.5, 1.0,        //specular
        0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    // zero for now
    osg::Matrixf light2(0.0, 0.0, 0.0, 1.0,        //ambient
        0.0, 0.0, 0.0, 1.0,        //diffuse
        0.0, 0.0, 0.0, 1.0,        //specular
        0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    osg::ref_ptr<osg::StateSet> rootState = m_root->getOrCreateStateSet();
    rootState->addUniform(new osg::Uniform("lightambientglobal", lightambientglobal));
    rootState->addUniform(new osg::Uniform("lightdir1", lightdir1));
    rootState->addUniform(new osg::Uniform("lightdir2", lightdir2));
    rootState->addUniform(new osg::Uniform("light1", light1));
    rootState->addUniform(new osg::Uniform("light2", light2));
}


osg::Group* World::root()
{
    return m_root.get();
}

void World::makeStandardBall()
{
    osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);

    // prototype: hard-coded physx values etc.
    //objects_container->makeStandardBall(m_particleGroup, physx::PxVec3(centerd.x(), centerd.y()+0.5, centerd.z()), 0.2F, physx::PxVec3(-2, 4, 0), physx::PxVec3(6, 13, 1));
    objects_container->makeParticleEmitter(m_particleGroup, physx::PxVec3(centerd.x(), centerd.y() + 0.5, centerd.z()));
    
}

void World::toogleBackgroundSound(int id){
    soundManager->togglePause(id);
}

void World::setNavigation(GodNavigation * navigation)
{
    m_navigation = navigation;
}

void World::initShader()
{
    osg::ref_ptr<osg::Shader> terrainBaseVertex =
        osgDB::readShaderFile("shader/terrain_base.vert");
    osg::ref_ptr<osg::Shader> terrainWaterVertex =
        osgDB::readShaderFile("shader/terrain_water.vert");
    osg::ref_ptr<osg::Shader> terrainBaseGeo = 
        osgDB::readShaderFile("shader/terrain_base.geom");
    osg::ref_ptr<osg::Shader> terrainWaterGeo = 
        osgDB::readShaderFile("shader/terrain_water.geom");
    osg::ref_ptr<osg::Shader> terrainBaseFragment =
        osgDB::readShaderFile("shader/terrain_base.frag");
    osg::ref_ptr<osg::Shader> terrainWaterFragment =
        osgDB::readShaderFile("shader/terrain_water.frag");
    osg::ref_ptr<osg::Shader> phongLightningFragment =
        osgDB::readShaderFile("shader/phongLighting.frag");

    assert(terrainBaseVertex.valid() && terrainBaseGeo.valid() && terrainBaseFragment.valid());
    assert(terrainWaterVertex.valid() && terrainWaterGeo.valid() && terrainWaterFragment.valid());
    assert(terrainWaterFragment.valid());

    osg::ref_ptr<osg::Program> terrainBaseProgram = new osg::Program();
    m_programsByName.emplace("terrainBase", terrainBaseProgram.get());
    terrainBaseProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    terrainBaseProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    terrainBaseProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    terrainBaseProgram->addShader(terrainBaseVertex);
    terrainBaseProgram->addShader(terrainBaseGeo);
    terrainBaseProgram->addShader(terrainBaseFragment);
    terrainBaseProgram->addShader(phongLightningFragment);

    osg::ref_ptr<osg::Program> terrainWaterProgram = new osg::Program();
    m_programsByName.emplace("terrainWater", terrainWaterProgram.get());
    terrainWaterProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    terrainWaterProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    terrainWaterProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    terrainWaterProgram->addShader(terrainWaterVertex);
    terrainWaterProgram->addShader(terrainWaterGeo);
    terrainWaterProgram->addShader(terrainWaterFragment);
    terrainWaterProgram->addShader(phongLightningFragment);


    terrain->osgTerrainBase()->getOrCreateStateSet()->setAttributeAndModes(terrainBaseProgram.get());
    terrain->osgTerrainWater()->getOrCreateStateSet()->setAttributeAndModes(terrainWaterProgram.get());

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

void World::setUniforms(long double globalTime)
{
    assert(!m_programsByName.empty()); // we don't want to set uniforms when we do not have shaders
    assert(m_navigation.valid());

    osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);
    osg::Vec3f eye(eyed);
    osg::ref_ptr<osg::StateSet> rootStateSet = m_root->getOrCreateStateSet();
    rootStateSet->getOrCreateUniform("cameraposition", osg::Uniform::FLOAT_VEC3)->set(eye);
    rootStateSet->getOrCreateUniform("globalTime", osg::Uniform::FLOAT)->set(
        static_cast<float>(globalTime));    // cast away the high precision, as not needed in the shaders
    rootStateSet->getOrCreateUniform("gameTime", osg::Uniform::FLOAT)->set(
        static_cast<float>(physics_wrapper->currentTime()));    // cast away the high precision, as not needed in the shaders

    // float height = terrain->heightAt(centerd.x(), centerd.z());
    // height + cone height
    // osg::Vec3 cameraCenter(centerd.x(), centerd.y(), centerd.z());

    osg::Matrix camDebuggerTransform = osg::Matrix::rotate(3.1415926f * 0.5, osg::Vec3(1.0, .0, .0))
        * osg::Matrix::translate( centerd );
    m_cameraDebugger->setMatrix(camDebuggerTransform);

    m_cameraDebugger->getOrCreateStateSet()->getOrCreateUniform("modelRotation",
        osg::Uniform::Type::FLOAT_MAT4)->set(osg::Matrixf::rotate(3.1415926f * 0.5, osg::Vec3f(1.0, .0, .0)));
}
