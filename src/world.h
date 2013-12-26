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
class SoundManager;
class CyclicTime;

class World {
public:

    World();
    ~World();

    /** Pauses physics updates, causing the game to be 'freezed' (the navigation etc. will work though). */
    void togglePause();

    void stopSimulation();
    void update();

    /** Throws a standard osg ball into the game using the PhysicsWrapper with correct physics.*/
    void makeStandardBall(const glm::vec3& position);
    void createFountainSound(const glm::vec3& position);

    /** plays and pauses the background sound **/
    void toggleBackgroundSound(int id);

    void updateListener();

    void setNavigation(Navigation & navigation);

    void setUniforms(glow::Program & program);

    glow::Program * programByName(const std::string & name);
    
    //std::shared_ptr<ElemateHeightFieldTerrain>  terrain;
    std::shared_ptr<SoundManager>               m_soundManager;

protected:
    std::shared_ptr<PhysicsWrapper>             m_physicsWrapper;

    Navigation * m_navigation;
    CyclicTime * m_time;
    std::unordered_map<std::string, glow::ref_ptr<glow::Program>> m_programsByName;

    void setUpLighting(glow::Program & program);
    void initShader();
};
