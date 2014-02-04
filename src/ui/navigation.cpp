#include "navigation.h"

#include <glow/logging.h>
#include "cameraex.h"

#include <GLFW/glfw3.h>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "terrain/terrain.h"


static const float c_distanceEyeCenterDefault = 15.f;
static const float c_speedScale = 0.05f;


Navigation::Navigation(GLFWwindow & window, const std::shared_ptr<CameraEx> & camera, const std::shared_ptr<Terrain> & terrain)
: m_window(window)
, m_camera(camera)
, m_terrain(terrain)
, m_center()
, m_distanceEyeCenter(c_distanceEyeCenterDefault)  
{
    setTransformation(glm::vec3(116.415, 16.149, 111.774), glm::vec3(104.737, 8.405, 97.194), glm::vec3(0, 1, 0));
}

Navigation::~Navigation()
{
}

void Navigation::setTransformation(const glm::vec3 & eye, const glm::vec3 & center, const glm::vec3 & /*up*/)
{
    glm::vec3 center_terrainHeight = glm::vec3(
        center.x,
        m_terrain->heightTotalAt(center.x, center.z),
        center.z);

    m_center = center_terrainHeight;
    m_lookAtVector = center_terrainHeight - eye;
    m_distanceEyeCenter = glm::length(m_lookAtVector);

    apply();
}

void Navigation::setTransformation(const glm::vec3 & center, const glm::vec3 & lookAtVector, const float & distance)
{
    glm::vec3 center_terrainHeight = glm::vec3(
        center.x,
        m_terrain->heightTotalAt(center.x, center.z),
        center.z);

    m_center = center_terrainHeight;
    m_lookAtVector = lookAtVector;
    m_distanceEyeCenter = distance;

    apply();
}

void Navigation::handleScrollEvent(const double & /*xoffset*/, const double & yoffset)
{
    if (glfwGetKey(&m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        return;     // currently used for terrain manipulation
    
    if (glfwGetKey(&m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        if (yoffset > 0)
        {
            if (m_distanceEyeCenter <= 2.f) return;
            m_distanceEyeCenter -= 0.5f;
        } else {
            m_distanceEyeCenter += 0.5f;
        }
    } else {
        glm::vec3 eye = m_camera->eye();
        if (yoffset < 0)
        {
            if (eye.y <= m_terrain->heightTotalAt(eye.x, eye.z) + 1.f) return;
            pitch(2.f);
        } else {
            if ((eye - m_center).y >= m_distanceEyeCenter - (m_distanceEyeCenter/c_distanceEyeCenterDefault)) return;
            pitch(-2.f);
        }
    }
}

void Navigation::handleKeyEvent(const int & key, const int & /*scancode*/, const int & action, const int & /*mods*/)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        setTransformation(glm::vec3(116.415, 16.149, 111.774), glm::vec3(104.737, 8.405, 97.194), glm::vec3(0, 1, 0));
}

void Navigation::update(double delta)
{
    float frameScale = static_cast<float>(delta * 100);
    if (!glfwGetWindowAttrib(&m_window, GLFW_FOCUSED))
        return;

    glm::vec3 newCenter = m_center;
    glm::vec3 resultCenter = m_center;
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

    if (newCenter != m_center) {
        resultCenter += glm::normalize(newCenter - m_center) * c_speedScale * boost * frameScale;
        resultCenter = glm::vec3(
            resultCenter.x,
            m_terrain->heightTotalAt(resultCenter.x, resultCenter.z),
            resultCenter.z);

        if (m_terrain->validBoundingBox().inside(resultCenter))
            m_center = resultCenter;
    }
}

void Navigation::apply()
{
    glm::vec3 eye = m_center - glm::normalize(m_lookAtVector) * m_distanceEyeCenter;

    float eyeHeight = m_terrain->heightTotalAt(eye.x, eye.z);
    if (eye.y < eyeHeight + 1.f)
    {
        eye.y = eyeHeight + 1.f;
        m_lookAtVector = m_center - eye;
        m_distanceEyeCenter = glm::length(m_lookAtVector);
    }

    m_camera->setEye(eye);
    m_camera->setCenter(m_center);
    m_camera->setUp(glm::vec3(0, 1, 0));
}

void Navigation::move(glm::vec3 & position, const glm::vec3 & direction)
{
    glm::vec3 moveDirection = glm::rotateY(m_lookAtVector, -90 * direction.x);
    moveDirection += -2.f * moveDirection * direction.z;

    position += glm::vec3(moveDirection.x, 0, moveDirection.z);
}

void Navigation::rotate(const float & angle)
{
    m_lookAtVector = glm::rotateY(m_lookAtVector, angle);
}

void Navigation::pitch(const float & angle)
{
    m_lookAtVector = glm::rotate(m_lookAtVector, angle, glm::cross(m_lookAtVector, glm::vec3(0, 1, 0)));
}

const CameraEx & Navigation::camera() const
{
    return *m_camera.get();
}

float Navigation::rotationAngle() const
{
    float angle = glm::angle(glm::normalize(glm::vec3(m_lookAtVector.x, 0.f, m_lookAtVector.z)), glm::vec3(0.f, 0.f, -1.f));

    return m_lookAtVector.x <= 0.f ? angle : 360.f - angle;
}
