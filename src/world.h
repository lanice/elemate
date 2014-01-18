#pragma once

#include <glow/ref_ptr.h>

#include <memory>
#include <unordered_map>
#include <vector>
#include <list>

#include <glm/glm.hpp>

namespace glow {
    class Program;
}

class Navigation;
class PhysicsWrapper;
class SoundManager;
class CyclicTime;
class Hand;
class Terrain;

class World {
public:

    World(PhysicsWrapper & physicsWrapper);
    ~World();

    static World * getInstance();

    /** Pauses physics updates, causing the game to be 'freezed' (the navigation etc. will work though). */
    void togglePause();

    void stopSimulation();
    void update();

    /** Throws a standard osg ball into the game using the PhysicsWrapper with correct physics.*/
    void makeElements(const glm::vec3& position);
    void createFountainSound(const glm::vec3& position);
    
    void updateEmitterPosition(const glm::vec3& position);
    void selectNextEmitter();
    void startEmitting();
    void stopEmitting();

    /** plays and pauses the background sound **/
    void toggleBackgroundSound(int id);

    void updateListener();
    void reloadLua();

    void setNavigation(Navigation & navigation);

    const glm::vec3 & sunlightInvDirection() const;
    const glm::mat4 & sunlighting() const;
    void setUpLighting(glow::Program & program) const;

    glow::Program * programByName(const std::string & name);
    
    std::shared_ptr<Hand>                       hand;
    std::shared_ptr<Terrain>                    terrain;
    std::shared_ptr<SoundManager>               m_soundManager;

protected:
    static World * s_instance;

    PhysicsWrapper & m_physicsWrapper;
    std::list<std::string> m_currentElements;

    Navigation * m_navigation;
    std::shared_ptr<CyclicTime> m_time;
    std::unordered_map<std::string, glow::ref_ptr<glow::Program>> m_programsByName;

    std::vector<int> m_sounds;

    void initShader();

    glm::vec3 m_sunlightInvDirection;
    glm::mat4 m_sunlighting;

public:
    World(World&) = delete;
    void operator=(World&) = delete;
};
