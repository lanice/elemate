#include "navigation.h"

#include <glow/logging.h>
#include <glowutils/Camera.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>


static const double c_distanceEyeCenterDefault = 5.;


Navigation::Navigation(GLFWwindow & window, glowutils::Camera * camera) :
    m_window(window),
    m_camera(camera),
    m_distanceEyeCenter(c_distanceEyeCenterDefault)
{
    setTransformation(glm::vec3(0, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // eye, center, up
}

Navigation::~Navigation()
{
}


void Navigation::setTransformation(const glm::vec3 & eye, const glm::vec3 & center, const glm::vec3 & up)
{
    // GLM uses a different coordinate system: x is right, y is forward, z is up!
    // Therefore we have to adjust the vec3s.
    glm::mat4 lookAtMatrix(glm::lookAt(glm::vec3(eye.x, -eye.z, eye.y), glm::vec3(center.x, -center.z, center.y), glm::vec3(up.x, -up.z, up.y)));
    m_center = center;
    m_rotation = glm::toQuat(lookAtMatrix);

    apply();
}

void Navigation::update()
{
    if (glfwGetWindowAttrib(&m_window, GLFW_FOCUSED))
    {
        if (glfwGetKey(&m_window, GLFW_KEY_W) == GLFW_PRESS)
            move(glm::vec3(0, 0, -1));
        if (glfwGetKey(&m_window, GLFW_KEY_A) == GLFW_PRESS)
            move(glm::vec3(-1, 0, 0));
        if (glfwGetKey(&m_window, GLFW_KEY_S) == GLFW_PRESS)
            move(glm::vec3(0, 0, 1));
        if (glfwGetKey(&m_window, GLFW_KEY_D) == GLFW_PRESS)
            move(glm::vec3(1, 0, 0));
    }
}

void Navigation::apply()
{
    glm::vec3 eye = m_center + m_rotation * glm::vec3(0, 0, m_distanceEyeCenter);
    glm::vec3 up = m_rotation * glm::vec3(0, 1, 0);

    m_camera->setEye(eye);
    m_camera->setCenter(m_center);
    m_camera->setUp(up);
}

void Navigation::move(const glm::vec3 & direction)
{
    m_center += direction;
}

const glowutils::Camera * Navigation::camera() const
{
    return m_camera;
}
