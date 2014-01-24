#pragma once

#include <glow/ref_ptr.h>
#include <GL/glew.h>

#include <memory>
#include <unordered_map>
#include <vector>
#include <list>

#include <glm/glm.hpp>

namespace glow {
    class Shader;
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

    static World * instance();

    /** Pauses physics updates, causing the game to be 'freezed' (the navigation etc. will work though). */
    void togglePause();

    void stopSimulation();

    /** updates the physics but doesn't copy/update any datastructures */
    void updatePhysics();

    /** updates the world as needed for visualization and interaction */
    void updateVisuals();

    /** plays and pauses the background sound **/
    void toggleBackgroundSound(int id);

    void reloadLua();

    void setNavigation(Navigation & navigation);

    const glm::vec3 & sunPosition() const;
    const glm::mat4 & sunlight() const;
    void setUpLighting(glow::Program & program) const;

    /** The world maintains a list of shaders that are needed multiple times in the game (phonglighting, depth_util..).
      * Request these shaders here by there filename, just as you would do with glowutils. */
    glow::Shader * sharedShader(GLenum type, const std::string & filename) const;
    
    std::shared_ptr<Hand>                       hand;
    std::shared_ptr<Terrain>                    terrain;
    std::shared_ptr<SoundManager>               m_soundManager;

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
    void initShader();

    glm::vec3 m_sunPosition;
    glm::mat4 m_sunlight;

public:
    World(World&) = delete;
    void operator=(World&) = delete;
};
