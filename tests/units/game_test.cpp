#include <gtest/gtest.h>

#include <glow/global.h>
#include <GLFW/glfw3.h>

#include "game.h"


class Game_tests : public ::testing::Test {
protected:
    virtual void SetUp() override
    {
        m_game = nullptr;
    }
    virtual void TearDown() override
    {
        // just to clean up if a test failed
        delete m_game;
    }

    Game * m_game;
};

TEST_F(Game_tests, game_create)
{
    ASSERT_TRUE(glfwInit() == GL_TRUE);

    GLFWwindow * window = glfwCreateWindow(1, 1, "elemateTest", NULL, NULL);
    ASSERT_TRUE(window != NULL);

    glfwMakeContextCurrent(window);

    ASSERT_TRUE(glow::init());

    EXPECT_NO_FATAL_FAILURE(m_game = new Game(*window));

    delete m_game;

    glfwTerminate();
}
