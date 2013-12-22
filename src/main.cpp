// These includes cause the '[..] needs to have dll-interface [..]' warnings.
// Seems it is still a problem? See https://github.com/hpicgs/glow/issues/14
#include <glow/global.h>
#include <glow/Error.h>
#include <glow/logging.h>
#include <glowutils/FileRegistry.h>

#include <GLFW/glfw3.h>

#include "game.h"

static GLint MajorVersionRequire = 3;
static GLint MinorVersionRequire = 3;

static Game * game;

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
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        // glfwSetWindowShouldClose(window, GL_TRUE);
        game->end();
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        glow::info("Updating shader...");
        glowutils::FileRegistry::instance().reloadAll();
        glow::info("Updating shader done.");
    }

    game->eventHandler()->handleKeyEvent(key, scancode, action, mods);
}

static void scrollCallback(GLFWwindow* /*window*/, double xoffset, double yoffset)
{
    // As soon as we have a HUD interface in which we scroll,
    // e.g. to choose an element, add a condition here,
    // as we don't want the navigation to scroll if we are "navigating" in our HUD.

    // if (HUD_active)
    // {
    //     game->hud()->handleScrollEvent(xoffset, yoffset);
    //     return;
    // }

    // or something like this...

    game->navigation()->handleScrollEvent(xoffset, yoffset);
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

    checkVersion();

    // GLOW takes care of initializing GLEW correctly.
    if (!glow::init())
    {
        glow::fatal("GLOW initialization failed.");
        return -1;
    }
    

    game = new Game(*window);

    game->start();

    glfwTerminate();

    return 0;
}
