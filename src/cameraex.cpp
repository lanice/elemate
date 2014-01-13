#include "cameraex.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>


using namespace glm;


CameraEx::CameraEx(
const vec3 & eye
, const vec3 & center
, const vec3 & up)
: glowutils::Camera(eye, center, up)

, m_zNearOrtho(-10.0f)
, m_left(-10.0f)
, m_right(10.0f)
, m_bottom(-10.0f)
, m_top(10.0f)
{
}

void CameraEx::changed() const
{
    Camera::changed();
    m_projectionOrthographic.invalidate();
    m_projectionOrthographicInverted.invalidate();
    m_viewProjectionOrthographic.invalidate();
    m_viewProjectionOrthographicInverted.invalidate();
}


float CameraEx::zNearOrtho() const
{
    return m_zNearOrtho;
}

void CameraEx::setZNearOtho(float zNear)
{
    if (zNear == m_zNearOrtho)
        return;

    m_zNearOrtho = zNear;

    dirty();
}

float CameraEx::left() const
{
    return m_left;
}

void CameraEx::setLeft(float left)
{
    if (left == m_left)
        return;

    m_left = left;

    dirty();
}

float CameraEx::right() const
{
    return m_right;
}

void CameraEx::setRight(float right)
{
    if (right == m_right)
        return;

    m_right = right;

    dirty();
}

float CameraEx::bottom() const
{
    return m_bottom;
}

void CameraEx::setBottom(float bottom)
{
    if (bottom == m_bottom)
        return;

    m_bottom = bottom;

    dirty();
}

float CameraEx::top() const
{
    return m_top;
}

void CameraEx::setTop(float top)
{
    if (top == m_top)
        return;

    m_top = top;

    dirty();
}

const mat4 & CameraEx::projectionOrthographic() const
{
    if (m_dirty)
        update();

    if (!m_projectionOrthographic.isValid())
        m_projectionOrthographic.setValue(ortho<float>(m_left, m_right, m_bottom, m_top, m_zNearOrtho, m_zFar));
    return m_projectionOrthographic.value();
}

const mat4 & CameraEx::viewProjectionOrthographic() const
{
    if (m_dirty)
        update();

    if (!m_viewProjectionOrthographic.isValid())
        m_viewProjectionOrthographic.setValue(projectionOrthographic() * view());

    return m_viewProjectionOrthographic.value();
}

const mat4 & CameraEx::projectionOrthographicInverted() const
{
    if (m_dirty)
        update();

    if (!m_projectionOrthographicInverted.isValid())
        m_projectionOrthographicInverted.setValue(inverse(projectionOrthographic()));

    return m_projectionOrthographicInverted.value();
}

const mat4 & CameraEx::viewProjectionOrthographicInverted() const
{
    if (m_dirty)
        update();

    if (!m_viewProjectionOrthographicInverted.isValid())
        m_viewProjectionOrthographicInverted.setValue(inverse(viewProjectionOrthographic()));

    return m_viewProjectionOrthographicInverted.value();
}
