#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);

#include <memory> //shared_ptr
#include <map>
#include <osg/ref_ptr>

class World;
class GodNavigation;
class CyclicTime;
namespace std {         class thread; }
namespace osgViewer {   class Viewer; }
namespace osg {
    class Camera;
    class Texture2D;
}

/** The Game Class that invokes a game loop and initializes PhysX.
 *  To receive the initialized physics, call getPhysicsWrapper(). See for its usage the documentation of PhysicsWrapper class.
 *  Typical Workflow is instantiating the Game class, call start() and let it run until the game ends, then call end().
 */
class Game{
public:

    /** Explicit Constructor because Copying and Assignments are disabled. */
    Game() = delete;
    explicit Game(osgViewer::Viewer& viewer);

    ~Game();

    /** Starts the Game Loop until end() is called.  */
    void start();

    /** True if the game loop is running. */
    bool isRunning() const;

    /** Ending the loop. */
    void end();
protected:

    /** The Game's loop containing drawing and triggering physics is placed right here.
      * @param delta specifies the time between each logic update in seconds.*/
    void loop(long double delta = 1.0L/100.0L);

    /** Initializes the rendering pipeline. Call after World::reloadShaders for access to the flush program. */
    void initRendering();
    void setupNavigation();
    void setLightSource();
    void generateTerrain();

    
    osgViewer::Viewer&          m_viewer;
    osg::ref_ptr<osg::Camera>   m_mainCamera;
    osg::ref_ptr<osg::Camera>   m_flushCamera;
    std::map<std::string, osg::ref_ptr<osg::Texture2D>> m_renderBuffers;
    std::shared_ptr<World>      m_world;
    std::shared_ptr<CyclicTime> m_cyclicTime;

    bool                        m_interrupted;
    osg::ref_ptr<GodNavigation> m_navigation;

    friend struct GameResizeCallback;

private:
    DISALLOW_COPY_AND_ASSIGN(Game);
};