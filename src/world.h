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
class ParticleGroup;

class World {
public:

    World(PhysicsWrapper & physicsWrapper);
    ~World();

    /** Pauses physics updates, causing the game to be 'freezed' (the navigation etc. will work though). */
    void togglePause();

    void stopSimulation();

    /** updates the physics but doesn't copy/update any datastructures */
    void updatePhysics();

    /** updates the world as needed for visualization and interaction */
    void updateVisuals();

    void notifyParticleGroups();
    void notifyParticleGroups(const double & delta);
    void registerObserver(ParticleGroup * observer);
    void unregisterObserver(ParticleGroup * observer);

    /** Throws a standard osg ball into the game using the PhysicsWrapper with correct physics.*/
    void makeElements(const glm::vec3& position);
    void createFountainSound(const glm::vec3& position);
    
    void updateEmitterPosition(const glm::vec3& position);
    void selectNextEmitter();
    void startEmitting();
    void stopEmitting();

    /** plays and pauses the background sound **/
    void toggleBackgroundSound(int id);

    void setNavigation(Navigation & navigation);

    const glm::vec3 & sunlightInvDirection() const;
    const glm::mat4 & sunlighting() const;
    void setUpLighting(glow::Program & program) const;

    glow::Program * programByName(const std::string & name);
    
    std::shared_ptr<Hand>                       hand;
    std::shared_ptr<Terrain>                    terrain;
    std::shared_ptr<SoundManager>               m_soundManager;

protected:
    PhysicsWrapper & m_physicsWrapper;
    std::list<std::string> m_currentElements;

    Navigation * m_navigation;
    std::shared_ptr<CyclicTime> m_time;
    std::unordered_map<std::string, glow::ref_ptr<glow::Program>> m_programsByName;

    std::vector<int> m_sounds;

    void updateListener();
    void initShader();

    glm::vec3 m_sunlightInvDirection;
    glm::mat4 m_sunlighting;

    std::vector<ParticleGroup *> m_particleGroupObservers;

public:
    World(World&) = delete;
    void operator=(World&) = delete;
};
