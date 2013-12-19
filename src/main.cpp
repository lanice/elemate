#include <GLFW/glfw3.h>

#include <iostream>

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
        std::cerr << "Could not initialize glfw." << std::endl;
        return -1;
    }

    GLFWwindow * window = glfwCreateWindow(640, 480, "Elemate", NULL, NULL);
    if (!window)
    {
        std::cerr << "glfw window creation failed." << std::endl;
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
