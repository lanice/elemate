#pragma once

#include <glow/ref_ptr.h>

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

namespace glow {
    class Program;
}

class Navigation;
class PhysicsWrapper;
class ObjectsContainer;
class SoundManager;
class CyclicTime;

class World {
public:

    World();
    ~World();

    void startSimulation();
    void stopSimulation();
    void update();

    /** Throws a standard osg ball into the game using the ObjectsContainer with correct physics.*/
    void makeStandardBall(const glm::vec3& position);
    void createFountainSound();

    /** plays and pauses the background sound **/
    void toggleBackgroundSound(int id);

    void setNavigation(Navigation & navigation);

    void setUniforms(glow::Program & program);

    glow::Program * programByName(const std::string & name);
    
    //std::shared_ptr<ElemateHeightFieldTerrain>  terrain;
    std::shared_ptr<SoundManager>               soundManager;

protected:
    std::shared_ptr<PhysicsWrapper>             m_physicsWrapper;
    std::shared_ptr<ObjectsContainer>           m_objectsContainer;

    Navigation * m_navigation;
    CyclicTime * m_time;
    std::unordered_map<std::string, glow::ref_ptr<glow::Program>> m_programsByName;

    void setUpLighting(glow::Program & program);
    void initShader();
};
