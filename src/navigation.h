#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>


namespace glowutils { class Camera; }

class Navigation
{
public:
    Navigation(GLFWwindow & window, glowutils::Camera * camera);
    virtual ~Navigation();

    void update();

    const glowutils::Camera * camera() const;


protected:
    GLFWwindow & m_window;
    glowutils::Camera * m_camera;

    void operator= (Navigation&) = delete;
};
