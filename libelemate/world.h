#pragma once

#include <glow/ref_ptr.h>
#include <GL/glew.h>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>

#include <glm/glm.hpp>

namespace glow {
    class Shader;
    class Program;
}

class Navigation;
class PhysicsWrapper;
class CyclicTime;
class Hand;
class Terrain;
class ParticleGroup;
class LuaWrapper;

class World {
public:

    World(PhysicsWrapper & physicsWrapper);
    ~World();

    static World * instance();

    /** Pauses physics updates, causing the game to be 'frozen' (the navigation etc. will work though). */
    void togglePause();

    time_t getTime() const;

    void stopSimulation();

    /** updates the physics but doesn't copy/update any datastructures */
    void updatePhysics();

    /** updates the world as needed for visualization and interaction */
    void updateVisuals();

    void registerObserver(ParticleGroup * observer);
    void unregisterObserver(ParticleGroup * observer);

    void createFountainSound(const glm::vec3& position);
    
    /** plays and pauses the background sound **/
    void toggleBackgroundSound(int id);

    void setNavigation(Navigation & navigation);

    const glm::vec3 & sunPosition() const;
    const glm::mat4 & sunlight() const;
    void setUpLighting(glow::Program & program) const;
    const glm::vec3 & skyColor() const;

    void registerLuaFunctions(LuaWrapper * lua);

    /** The world maintains a list of shaders that are needed multiple times in the game (phongLighting, depth_util..).
      * Request these shaders here by there filename, just as you would do with glowutils. */
    glow::Shader * sharedShader(GLenum type, const std::string & filename) const;
    
    std::shared_ptr<Hand>                       hand;
    std::shared_ptr<Terrain>                    terrain;

protected:
    static World * s_instance;

    PhysicsWrapper & m_physicsWrapper;
    std::list<std::string> m_currentElements;

    Navigation * m_navigation;
    std::shared_ptr<CyclicTime> m_time;

    /** shaders that are needed multiple times in the game.
      * This is mutable, so that you can use the lazy sharedShader getter in const functions. */
    mutable std::unordered_map<std::string, glow::ref_ptr<glow::Shader>> m_sharedShaders;

    std::vector<int> m_sounds;

    void updateListener();

    glm::vec3 m_sunPosition;
    glm::mat4 m_sunlight;
    glm::vec3 m_skyColor;

    std::unordered_set<ParticleGroup *> m_particleGroupObservers;

public:
    World(World&) = delete;
    void operator=(World&) = delete;
};
