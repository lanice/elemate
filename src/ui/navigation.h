#pragma once

#include <memory>

#include <glow/global.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>


class Terrain;
class CameraEx;

class Navigation
{
public:
    Navigation(GLFWwindow & window, const std::shared_ptr<CameraEx> & camera, const std::shared_ptr<Terrain> & terrain);
    virtual ~Navigation();

    void setTransformation(const glm::vec3 & eye, const glm::vec3 & center, const glm::vec3 & up);
    void setTransformation(const glm::vec3 & center, const glm::vec3 & lookAtVector, const float & distance);

    void handleScrollEvent(const double & xoffset, const double & yoffset);
    void handleKeyEvent(const int & key, const int & scancode, const int & action, const int & mods);

    void update(double delta);
    void apply();

    const CameraEx & camera() const;

    float rotationAngle() const;


protected:
    void move(glm::vec3 & position, const glm::vec3 & direction);
    void rotate(const float & degree);
    void pitch(const float & degree);

    GLFWwindow & m_window;
    std::shared_ptr<CameraEx> m_camera;
    std::shared_ptr<Terrain> m_terrain;

    glm::vec3 m_center;
    glm::vec3 m_lookAtVector;
    float m_distanceEyeCenter;

public:
    Navigation() = delete;
    void operator= (Navigation&) = delete;
};
