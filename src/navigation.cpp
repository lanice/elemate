#include "navigation.h"

#include <glow/logging.h>
#include <glowutils/Camera.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/quaternion.hpp>


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
    glm::mat4 lookAtMatrix(glm::lookAt(eye, center, up));
    m_center = center;
    m_rotation = glm::toQuat(lookAtMatrix);

    apply();
}

void Navigation::update()
{
    if (glfwGetWindowAttrib(&m_window, GLFW_FOCUSED))
    {
        if (glfwGetKey(&m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
            setTransformation(glm::vec3(0, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        if (glfwGetKey(&m_window, GLFW_KEY_W) == GLFW_PRESS)
            move(glm::vec3(0, 0, -1));
        if (glfwGetKey(&m_window, GLFW_KEY_A) == GLFW_PRESS)
            move(glm::vec3(-1, 0, 0));
        if (glfwGetKey(&m_window, GLFW_KEY_S) == GLFW_PRESS)
            move(glm::vec3(0, 0, 1));
        if (glfwGetKey(&m_window, GLFW_KEY_D) == GLFW_PRESS)
            move(glm::vec3(1, 0, 0));

        if (glfwGetKey(&m_window, GLFW_KEY_Q) == GLFW_PRESS)
            rotate(-1.);
        if (glfwGetKey(&m_window, GLFW_KEY_E) == GLFW_PRESS)
            rotate(1.);
    }
}

void Navigation::apply()
{
    glm::vec3 eye = m_center + m_rotation * glm::vec3(0, m_distanceEyeCenter, 0);

    m_camera->setEye(eye);
    m_camera->setCenter(m_center);
    m_camera->setUp(glm::vec3(0, 1, 0));
}

void Navigation::move(const glm::vec3 & direction)
{  
    m_center += glm::vec3(direction.x, 0, direction.z);
}

void Navigation::rotate(const float & angle)
{
    m_rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0)) * m_rotation;
}

const glowutils::Camera * Navigation::camera() const
{
    return m_camera;
}
