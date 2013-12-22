#include "world.h"

#include <stdexcept>

#include <glow/logging.h>
#include <glow/Program.h>
#include <glow/Shader.h>
#include <glowutils/File.h>
#include <glowutils/Camera.h>

#include "PxPhysicsAPI.h"

#include <glm/glm.hpp>

#include "physicswrapper.h"
#include "objectscontainer.h"
#include "soundmanager.h"
#include "navigation.h"
#include "elements.h"
#include "terrain/terraingenerator.h"
#include "terrain/terrain.h"


World::World()
: physicsWrapper(new PhysicsWrapper())
, objectsContainer(new ObjectsContainer(physicsWrapper))
, soundManager(new SoundManager())
, m_navigation(nullptr)
{    
    // Create two non-3D channels (paino and rain)
    //initialise as paused
    soundManager->createNewChannel("data/sounds/rain.mp3", true, false, true);
    soundManager->createNewChannel("data/sounds/piano.mp3", true, false, true);
    //set volume (make quieter)
    soundManager->setVolume(0, 0.14f);
    soundManager->setVolume(1, 0.3f);

    initShader();

    TerrainGenerator terrainGen;
    terrainGen.setExtentsInWorld(50, 50);
    terrainGen.applySamplesPerWorldCoord(1.f);
    terrainGen.setTilesPerAxis(1, 1);
    terrainGen.setMaxHeight(20.0f);
    terrainGen.setMaxBasicHeightVariance(0.05f);

    terrain = std::shared_ptr<Terrain>(terrainGen.generate());

    for (const auto actor : terrain->pxActorMap())
        physicsWrapper->scene()->addActor(*actor.second);
}


World::~World()
{
}

void World::makeStandardBall(const glm::vec3& position)
{
    objectsContainer->makeParticleEmitter(physx::PxVec3(position.x, position.y, position.z));
}

void World::createFountainSound()
{
    /*osg::Vec3d eyed, upd, centerd;
    m_navigation->getTransformation(eyed, centerd, upd);*/
    glm::vec3 center(0, 0, 0);
    soundManager->createNewChannel("data/sounds/fountain_loop.wav", true, true, false, { center.x, center.y + 0.5f, center.z });
}

void World::toogleBackgroundSound(int id){
    soundManager->togglePause(id);
}

void World::setNavigation(Navigation & navigation)
{
    m_navigation = &navigation;
}

void World::initShader()
{
}

void World::setUpLighting(glow::Program & program)
{
    static glm::vec4 lightambientglobal(0, 0, 0, 0);
    static glm::vec3 lightdir1(0.0, 6.5, 7.5);
    static glm::vec3 lightdir2(0.0, -8.0, 7.5);

    static glm::mat4 lightMat1;
    lightMat1[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    lightMat1[1] = glm::vec4(0.2, 0.2, 0.2, 1.0);        //diffuse
    lightMat1[2] = glm::vec4(0.7, 0.7, 0.5, 1.0);        //specular
    lightMat1[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    static glm::mat4 lightMat2;
    lightMat2[0] = glm::vec4(0.0, 0.0, 0.0, 1.0);        //ambient
    lightMat2[1] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //diffuse
    lightMat2[2] = glm::vec4(0.1, 0.1, 0.1, 1.0);        //specular
    lightMat2[3] = glm::vec4(0.002, 0.002, 0.0004, 1.4); //attenuation1, attenuation2, attenuation3, shininess

    program.setUniform("lightambientglobal", lightambientglobal);
    program.setUniform("lightdir1", lightdir1);
    program.setUniform("lightdir2", lightdir2);
    program.setUniform("light1", lightMat1);
    program.setUniform("light2", lightMat2);
}

void World::setUniforms(glow::Program & program)
{
    assert(m_navigation);
    program.setUniform("view", m_navigation->camera()->view());
    program.setUniform("viewProjection", m_navigation->camera()->viewProjection());
    program.setUniform("cameraposition", m_navigation->camera()->eye());
    
    setUpLighting(program);

    Elements::setAllUniforms(program);
}

glow::Program * World::programByName(const std::string & name)
{
    try {
        return m_programsByName.at(name).get();
    }
    catch (std::out_of_range e) {
        glow::critical("trying to use unloaded shader %;", name);
        return nullptr;
    }
}
