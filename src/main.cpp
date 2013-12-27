// These includes cause the '[..] needs to have dll-interface [..]' warnings.
// Seems it is still a problem? See https://github.com/hpicgs/glow/issues/14
#include <glow/global.h>
#include <glow/Error.h>
#include <glow/logging.h>

#include <GLFW/glfw3.h>

#include "game.h"
#include "eventhandler.h"

static GLint MajorVersionRequire = 3;
static GLint MinorVersionRequire = 3;

static EventHandler * eventHandler;

static void checkVersion() {
    glow::info("OpenGL Version Needed %;.%; (%;.%; Found)",
        MajorVersionRequire, MinorVersionRequire,
        glow::query::majorVersion(), glow::query::minorVersion());
    glow::info("version: %;", glow::query::version().toString());
    glow::info("vendor: %;", glow::query::vendor());
    glow::info("renderer: %;", glow::query::renderer());
    glow::info("core profile: %;", glow::query::isCoreProfile() ? "true" : "false");
    glow::info("GLSL version: %;\n", glow::query::getString(GL_SHADING_LANGUAGE_VERSION));
}

static void errorCallback(int /*error*/, const char* description)
{
    glow::warning(description);
}

static void keyCallback(GLFWwindow* /*window*/, int key, int scancode, int action, int mods)
{
    if (eventHandler)
        eventHandler->handleKeyEvent(key, scancode, action, mods);
}

static void scrollCallback(GLFWwindow* /*window*/, double xoffset, double yoffset)
{
    if (eventHandler)
        eventHandler->handleScrollEvent(xoffset, yoffset);
}

void setCallbacks(GLFWwindow * window)
{
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
}


int main()
{
    
    if (!glfwInit()) {
        glow::fatal("Could not initialize glfw.");
        return -1;
    }

    glfwSetErrorCallback(errorCallback);

    GLFWwindow * window = glfwCreateWindow(640, 480, "Elemate", NULL, NULL);
    if (!window)
    {
        glow::fatal("glfw window creation failed.");
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);
    setCallbacks(window);
    glfwSwapInterval(1);

    checkVersion();

    // GLOW takes care of initializing GLEW correctly.
    if (!glow::init())
    {
        glow::fatal("GLOW initialization failed.");
        return -1;
    }
    

    Game * game = new Game(*window);
    eventHandler = new EventHandler(*window, *game);

    game->start();

    glfwTerminate();

    return 0;
}
