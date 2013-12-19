#include "navigation.h"

#include <glow/logging.h>
#include <glowutils/Camera.h>


Navigation::Navigation(GLFWwindow & window, glowutils::Camera * camera) :
    m_window(window),
    m_camera(camera)
{
    camera->setCenter(glm::vec3(0, 0, 0));
    camera->setUp(glm::vec3(0, 1, 0));
    camera->setEye(glm::vec3(0, 2, 2));
}

Navigation::~Navigation()
{
}


void Navigation::update()
{
}

const glowutils::Camera * Navigation::camera() const
{
    return m_camera;
}
