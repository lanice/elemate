#pragma once

#include <glow/global.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


namespace glowutils { class Camera; }

class Navigation
{
public:
    Navigation(GLFWwindow & window, glowutils::Camera * camera);
    virtual ~Navigation();

    void setTransformation(const glm::vec3 & eye, const glm::vec3 & center, const glm::vec3 & up);

    void handleScrollEvent(const double & xoffset, const double & yoffset);
    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);

    void update(double delta);
    void apply();

    void move(glm::vec3 & position, const glm::vec3 & direction);
    void rotate(const float & degree);
    void pitch(const float & degree);

    const glowutils::Camera * camera() const;


protected:
    GLFWwindow & m_window;
    glowutils::Camera * m_camera;

    glm::vec3 m_center;
    glm::quat m_rotation;
    double m_distanceEyeCenter;

    void operator= (Navigation&) = delete;
};
