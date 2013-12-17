#include "world.h"

#include "PxPhysicsAPI.h"

#include <osgViewer/Viewer>
#include <osgTerrain/Terrain>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osg/Matrix>

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
    soundManager->setVolume(0, 0.14f);
    soundManager->setVolume(1, 0.3f);

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

    setUpLighting();
}


World::~World()
{
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

osg::Matrixd World::getCameraTransform()
{
    return m_navigation->getMatrix();
}

void World::makeStandardBall(const osg::Vec3d& position)
{
    osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);

    // prototype: hard-coded physx values etc.
    //objects_container->makeStandardBall(m_particleGroup, physx::PxVec3(centerd.x(), centerd.y()+0.5, centerd.z()), 0.2F, physx::PxVec3(-2, 4, 0), physx::PxVec3(6, 13, 1));
    //objects_container->createParticles(5, physx::PxVec3(centerd.x(), centerd.y() + 0.5, centerd.z()));
    objects_container->makeParticleEmitter(m_particleGroup, physx::PxVec3(position.x(), position.y(), position.z()));
}

void World::startFountainSound()
{
    osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);
    soundManager->createNewChannel("data/sounds/fountain_loop.wav", true, true, false, { centerd.x(), centerd.y() + 0.5, centerd.z() });
}

void World::updateFountainPosition()
{
    osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);
    soundManager->setSoundPos(2, { centerd.x(), centerd.y() + 0.5, centerd.z() });
    soundManager->update();
}

void World::endFountainSound()
{
    soundManager->deleteChannel(2);

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
    osg::ref_ptr<osg::Shader> flushVertex =
        osgDB::readShaderFile("shader/flush.vert");
    osg::ref_ptr<osg::Shader> flushFragment =
        osgDB::readShaderFile("shader/flush.frag");
    assert(flushVertex.valid() && flushFragment.valid());
    osg::ref_ptr<osg::Program> flushProgram = new osg::Program();
    flushProgram->addShader(flushVertex);
    flushProgram->addShader(flushFragment);
    m_programsByName.emplace("flush", flushProgram.get());


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

    osg::ref_ptr<osg::Shader> particleWaterVertex =
        osgDB::readShaderFile("shader/particle_water.vert");
    osg::ref_ptr<osg::Shader> particleWaterFragment =
        osgDB::readShaderFile("shader/particle_water.frag");
    assert(particleWaterVertex.valid() && particleWaterFragment.valid());

    osg::ref_ptr<osg::Program> particleWaterProgram = new osg::Program();
    m_programsByName.emplace("particle_water", particleWaterProgram.get());
    particleWaterProgram->addShader(particleWaterVertex);
    particleWaterProgram->addShader(particleWaterFragment);

    osg::ref_ptr<osg::StateSet> particleStateSet = m_particleGroup->getOrCreateStateSet();
    particleStateSet->setAttributeAndModes(particleWaterProgram.get());



    osg::ref_ptr<osg::Shader> handVertexShader =
        osgDB::readShaderFile("shader/hand.vert");
    osg::ref_ptr<osg::Shader> handFragmentShader =
        osgDB::readShaderFile("shader/hand.frag");
    assert(handVertexShader.valid() && handFragmentShader.valid());

    osg::ref_ptr<osg::Program> handProgram = new osg::Program();
    m_programsByName.emplace("hand", handProgram.get());
    handProgram->addShader(handVertexShader);
    handProgram->addShader(handFragmentShader);
    handProgram->addShader(phongLightningFragment);
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

osg::Program * World::programByName(const std::string & name) const
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
}
