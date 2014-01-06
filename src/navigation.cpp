#include "navigation.h"

#include <glow/logging.h>
#include <glowutils/Camera.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include "terrain/terrain.h"


static const double c_distanceEyeCenterDefault = 5.;
static const float c_speedScale = 0.05f;


Navigation::Navigation(GLFWwindow & window, glowutils::Camera & camera, std::shared_ptr<Terrain>& terrain) :
    m_window(window),
    m_camera(&camera),
    m_distanceEyeCenter(c_distanceEyeCenterDefault),
    m_terrain(terrain)
{
    setTransformation(glm::vec3(0, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // eye, center, up
}

Navigation::~Navigation()
{
}

void Navigation::setTransformation(const glm::vec3 & eye, const glm::vec3 & center, const glm::vec3 & up)
{
    glm::vec3 center_terrainHeight = glm::vec3(
        center.x,
        m_terrain->heightAt(center.x, center.z),
        center.z);

    glm::mat4 lookAtMatrix(glm::lookAt(eye, center_terrainHeight, up));
    m_center = center_terrainHeight;
    m_rotation = glm::toQuat(lookAtMatrix);

    apply();
}

void Navigation::handleScrollEvent(const double & /*xoffset*/, const double & yoffset)
{
    if (glfwGetKey(&m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        if (yoffset > 0)
        {
            if (m_distanceEyeCenter <= 2.) return;
            m_distanceEyeCenter -= 0.5;
        } else {
            m_distanceEyeCenter += 0.5;
        }
    } else {
        glm::vec3 eye = m_camera->eye();
        if (yoffset < 0)
        {
            if (eye.y <= (m_distanceEyeCenter/c_distanceEyeCenterDefault)) return;
            pitch(2.);
        } else {
            if ((eye - m_center).y >= m_distanceEyeCenter - (m_distanceEyeCenter/c_distanceEyeCenterDefault)) return;
            pitch(-2.);
        }
    }
}

void Navigation::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & /*mods*/)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        setTransformation(glm::vec3(0, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        m_distanceEyeCenter = c_distanceEyeCenterDefault;
    }
}

void Navigation::update(double delta)
{
    float frameScale = static_cast<float>(delta * 100);
    if (glfwGetWindowAttrib(&m_window, GLFW_FOCUSED))
    {
        glm::vec3 newCenter = m_center;
        float boost = 1.f;

        if (glfwGetKey(&m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            boost = 5.f;

        if (glfwGetKey(&m_window, GLFW_KEY_W) == GLFW_PRESS)
            move(newCenter, glm::vec3(0, 0, -1));
        if (glfwGetKey(&m_window, GLFW_KEY_A) == GLFW_PRESS)
            move(newCenter, glm::vec3(-1, 0, 0));
        if (glfwGetKey(&m_window, GLFW_KEY_S) == GLFW_PRESS)
            move(newCenter, glm::vec3(0, 0, 1));
        if (glfwGetKey(&m_window, GLFW_KEY_D) == GLFW_PRESS)
            move(newCenter, glm::vec3(1, 0, 0));

        if (glfwGetKey(&m_window, GLFW_KEY_Q) == GLFW_PRESS)
            rotate(-1.f * boost * frameScale);
        if (glfwGetKey(&m_window, GLFW_KEY_E) == GLFW_PRESS)
            rotate(1.f * boost * frameScale);


        if (newCenter != m_center)
        {
            m_center += glm::normalize(newCenter - m_center) * c_speedScale * boost * frameScale;

            m_center = glm::vec3(
                m_center.x,
                m_terrain->heightAt(m_center.x, m_center.z),
                m_center.z);
        }
    }
}

void Navigation::apply()
{
    glm::vec3 eye = m_center + m_rotation * glm::vec3(0, m_distanceEyeCenter, 0);

    m_camera->setEye(eye);
    m_camera->setCenter(m_center);
    m_camera->setUp(glm::vec3(0, 1, 0));
}

void Navigation::move(glm::vec3 & position, const glm::vec3 & direction)
{
    glm::vec3 playerDirection = m_rotation * direction;

    position += glm::vec3(playerDirection.x, 0, playerDirection.z);
}

void Navigation::rotate(const float & angle)
{
    m_rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0)) * m_rotation;
}

void Navigation::pitch(const float & angle)
{
    m_rotation = glm::angleAxis(angle, m_rotation * glm::vec3(1, 0, 0)) * m_rotation;
}

const glowutils::Camera * Navigation::camera() const
{
    return m_camera;
}
