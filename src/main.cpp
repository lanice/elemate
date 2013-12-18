#include <GLFW/glfw3.h>

#include <iostream>

#include "game.h"

int main()
{
    if (!glfwInit()) {
        std::cerr << "Could not initialize glfw." << std::endl;
        return -1;
    }

    GLFWwindow * window = glfwCreateWindow(640, 480, "Elemate", NULL, NULL);
    if (!window)
    {
        std::cout << "glfw window creation failed." << std::endl;
        glfwTerminate();
        return -1;
    }


    Game game(*window);

    game.start();

    glfwMakeContextCurrent(window);

    glfwTerminate();

    return 0;
}
