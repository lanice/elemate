#pragma once

#include <memory>

#include <glow/global.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


namespace glowutils { class Camera; }
class Terrain;

class Navigation
{
public:
    Navigation(GLFWwindow & window, glowutils::Camera & camera, std::shared_ptr<Terrain>& terrain);
    virtual ~Navigation();

    void setTransformation(const glm::vec3 & eye, const glm::vec3 & center, const glm::vec3 & up);
    void setTransformation(const glm::vec3 & center, const glm::vec3 & lookAtVector, const float & distance);

    void handleScrollEvent(const double & xoffset, const double & yoffset);
    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);

    void update(double delta);
    void apply();

    void move(glm::vec3 & position, const glm::vec3 & direction);
    void rotate(const float & degree);
    void pitch(const float & degree);

    const glowutils::Camera * camera() const;

    const float rotationAngle() const;


protected:
    GLFWwindow & m_window;
    glowutils::Camera * m_camera;
    std::shared_ptr<Terrain> m_terrain;

    glm::vec3 m_center;
    glm::vec3 m_lookAtVector;
    glm::quat m_rotation;
    float m_distanceEyeCenter;

public:
    Navigation() = delete;
    void operator= (Navigation&) = delete;
};
