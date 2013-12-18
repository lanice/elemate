#pragma once

#include <memory>
#include <map>

class World;
class CyclicTime;
namespace std {         class thread; }
struct GLFWwindow;

/** The Game Class that invokes a game loop and initializes PhysX.
 *  To receive the initialized physics, call getPhysicsWrapper(). See for its usage the documentation of PhysicsWrapper class.
 *  Typical Workflow is instantiating the Game class, call start() and let it run until the game ends, then call end().
 */
class Game{
public:
    Game(GLFWwindow & window);

    ~Game();

    /** Starts the Game Loop until end() is called.  */
    void start();

    /** True if the game loop is running. */
    bool isRunning() const;

    /** Ending the loop. */
    void end();
protected:

    GLFWwindow & m_window;

    /** The Game's loop containing drawing and triggering physics is placed right here.
      * @param delta specifies the time between each logic update in seconds.*/
    void loop(long double delta = 1.0L/100.0L);

    std::shared_ptr<World>      m_world;
    std::shared_ptr<CyclicTime> m_cyclicTime;

    bool                        m_interrupted;

private:
    Game() = delete;
    void operator=(Game &) = delete;
};