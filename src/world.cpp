#include "world.h"

#include <iostream>

#include "PxPhysicsAPI.h"

#include <glm/glm.hpp>

#include "physicswrapper.h"
#include "objectscontainer.h"
#include "soundmanager.h"


World::World()
: physics_wrapper(new PhysicsWrapper())
, objects_container(new ObjectsContainer(physics_wrapper))
, soundManager(new SoundManager())
{    
    // Create two non-3D channels (paino and rain)
    //initialise as paused
    soundManager->createNewChannel("data/sounds/rain.mp3", true, false, true);
    soundManager->createNewChannel("data/sounds/piano.mp3", true, false, true);
    //set volume (make quieter)
    soundManager->setVolume(0, 0.14f);
    soundManager->setVolume(1, 0.3f);

    setUpLighting();
}


World::~World()
{
}


void World::setUpLighting()
{
    glm::vec4 lightambientglobal(0, 0, 0, 0);
    glm::vec3 lightdir1(0.0, 6.5, 7.5);
    glm::vec3 lightdir2(0.0, -8.0, 7.5);


    // TODO : mirror matrices for glm!

    // some kind of sunlight..
    //osg::Matrixf light1(0.0, 0.0, 0.0, 1.0,        //ambient
    //    0.2, 0.2, 0.2, 1.0,        //diffuse
    //    0.7, 0.7, 0.5, 1.0,        //specular
    //    0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    //// zero for now
    //osg::Matrixf light2(0.0, 0.0, 0.0, 1.0,        //ambient
    //    0.0, 0.0, 0.0, 1.0,        //diffuse
    //    0.0, 0.0, 0.0, 1.0,        //specular
    //    0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess
}

void World::makeStandardBall(const glm::vec3& position)
{
    objects_container->makeParticleEmitter(physx::PxVec3(position.x, position.y, position.z));
}

void World::createFountainSound()
{
    /*osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);*/
    glm::vec3 center(0, 0, 0);
    soundManager->createNewChannel("data/sounds/fountain_loop.wav", true, true, false, { center.x, center.y + 0.5, center.z });
}

void World::toogleBackgroundSound(int id){
    soundManager->togglePause(id);
}

void World::initShader()
{
    //osg::ref_ptr<osg::Shader> flushVertex =
    //    osgDB::readShaderFile("shader/flush.vert");
    //osg::ref_ptr<osg::Shader> flushFragment =
    //    osgDB::readShaderFile("shader/flush.frag");
    //assert(flushVertex.valid() && flushFragment.valid());
    //osg::ref_ptr<osg::Program> flushProgram = new osg::Program();
    //flushProgram->addShader(flushVertex);
    //flushProgram->addShader(flushFragment);
    //m_programsByName.emplace("flush", flushProgram.get());


    //osg::ref_ptr<osg::Shader> terrainBaseVertex =
    //    osgDB::readShaderFile("shader/terrain_base.vert");
    //osg::ref_ptr<osg::Shader> terrainWaterVertex =
    //    osgDB::readShaderFile("shader/terrain_water.vert");
    //osg::ref_ptr<osg::Shader> terrainBaseGeo = 
    //    osgDB::readShaderFile("shader/terrain_base.geom");
    //osg::ref_ptr<osg::Shader> terrainWaterGeo = 
    //    osgDB::readShaderFile("shader/terrain_water.geom");
    //osg::ref_ptr<osg::Shader> terrainBaseFragment =
    //    osgDB::readShaderFile("shader/terrain_base.frag");
    //osg::ref_ptr<osg::Shader> terrainWaterFragment =
    //    osgDB::readShaderFile("shader/terrain_water.frag");
    //osg::ref_ptr<osg::Shader> phongLightningFragment =
    //    osgDB::readShaderFile("shader/phongLighting.frag");

    //assert(terrainBaseVertex.valid() && terrainBaseGeo.valid() && terrainBaseFragment.valid());
    //assert(terrainWaterVertex.valid() && terrainWaterGeo.valid() && terrainWaterFragment.valid());
    //assert(terrainWaterFragment.valid());

    //osg::ref_ptr<osg::Program> terrainBaseProgram = new osg::Program();
    //m_programsByName.emplace("terrainBase", terrainBaseProgram.get());
    //terrainBaseProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    //terrainBaseProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    //terrainBaseProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    //terrainBaseProgram->addShader(terrainBaseVertex);
    //terrainBaseProgram->addShader(terrainBaseGeo);
    //terrainBaseProgram->addShader(terrainBaseFragment);
    //terrainBaseProgram->addShader(phongLightningFragment);

    //osg::ref_ptr<osg::Program> terrainWaterProgram = new osg::Program();
    //m_programsByName.emplace("terrainWater", terrainWaterProgram.get());
    //terrainWaterProgram->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
    //terrainWaterProgram->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
    //terrainWaterProgram->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    //terrainWaterProgram->addShader(terrainWaterVertex);
    //terrainWaterProgram->addShader(terrainWaterGeo);
    //terrainWaterProgram->addShader(terrainWaterFragment);
    //terrainWaterProgram->addShader(phongLightningFragment);


    //terrain->osgTerrainBase()->getOrCreateStateSet()->setAttributeAndModes(terrainBaseProgram.get());
    //terrain->osgTerrainWater()->getOrCreateStateSet()->setAttributeAndModes(terrainWaterProgram.get());

    //osg::ref_ptr<osg::Shader> particleWaterVertex =
    //    osgDB::readShaderFile("shader/particle_water.vert");
    //osg::ref_ptr<osg::Shader> particleWaterFragment =
    //    osgDB::readShaderFile("shader/particle_water.frag");
    //assert(particleWaterVertex.valid() && particleWaterFragment.valid());

    //osg::ref_ptr<osg::Program> particleWaterProgram = new osg::Program();
    //m_programsByName.emplace("particle_water", particleWaterProgram.get());
    //particleWaterProgram->addShader(particleWaterVertex);
    //particleWaterProgram->addShader(particleWaterFragment);

    //osg::ref_ptr<osg::StateSet> particleStateSet = m_particleGroup->getOrCreateStateSet();
    //particleStateSet->setAttributeAndModes(particleWaterProgram.get());



    //osg::ref_ptr<osg::Shader> handVertexShader =
    //    osgDB::readShaderFile("shader/hand.vert");
    //osg::ref_ptr<osg::Shader> handFragmentShader =
    //    osgDB::readShaderFile("shader/hand.frag");
    //assert(handVertexShader.valid() && handFragmentShader.valid());

    //osg::ref_ptr<osg::Program> handProgram = new osg::Program();
    //m_programsByName.emplace("hand", handProgram.get());
    //handProgram->addShader(handVertexShader);
    //handProgram->addShader(handFragmentShader);
    //handProgram->addShader(phongLightningFragment);
}

void World::reloadShader()
{
    std::cout << "TODO: shader loading" << std::endl;
    //if (m_programsByName.empty())
    //    return initShader();

    //// reload all shader for all program from source
    //for (auto & pair : m_programsByName)
    //{
    //    for (unsigned i = 0; i < pair.second->getNumShaders(); ++i)
    //    {
    //        osg::Shader * shader = pair.second->getShader(i);
    //        shader->loadShaderSourceFromFile(shader->getFileName());
    //    }
    //}
}

void World::setUniforms(long double globalTime)
{

}
