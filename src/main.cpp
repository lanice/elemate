#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// These include causes the '[..] needs to have dll-interface [..]' warnings.
// Seems it is still a problem? See https://github.com/hpicgs/glow/issues/14
#include <glow/logging.h>

#include "game.h"


static Game * game;

static void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        // glfwSetWindowShouldClose(window, GL_TRUE);
        game->end();
}

void setCallbacks(GLFWwindow * window)
{
    glfwSetKeyCallback(window, keyCallback);
}


int main()
{
    if (!glfwInit()) {
        glow::fatal("Could not initialize glfw.");
        return -1;
    }

    GLFWwindow * window = glfwCreateWindow(640, 480, "Elemate", NULL, NULL);
    if (!window)
    {
        glow::fatal("glfw window creation failed.");
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);

    setCallbacks(window);

    game = new Game(*window);

    game->start();

    glfwTerminate();

    return 0;
}
