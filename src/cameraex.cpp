#include "cameraex.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>


using namespace glm;


CameraEx::CameraEx(ProjectionType projectionType, const vec3 & eye, const vec3 & center, const vec3 & up)
: glowutils::Camera(eye, center, up)
, m_projectionType(projectionType)
, m_left(-10.0f)
, m_right(10.0f)
, m_bottom(-10.0f)
, m_top(10.0f)
{
}

ProjectionType CameraEx::projectionType() const
{
    return m_projectionType;
}

void CameraEx::setProjectionType(ProjectionType projectionType)
{
    m_projectionType = projectionType;
    m_projection.invalidate();
    m_projectionInverted.invalidate();
    m_viewProjection.invalidate();
    m_viewProjectionInverted.invalidate();
}

void CameraEx::changed() const
{
    Camera::changed();
}


float CameraEx::zNearEx() const
{
    return m_zNear;
}

void CameraEx::setZNearEx(float zNear)
{
    if (zNear == m_zNear)
        return;

    m_zNear = zNear;

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

const mat4 & CameraEx::projectionEx() const
{
    if (m_dirty)
        update();

    switch (m_projectionType) {
    case ProjectionType::perspective:
        return Camera::projection();
    case ProjectionType::orthographic:
        if (!m_projection.isValid())
            m_projection.setValue(ortho<float>(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar));
        break;
    }
    return m_projection.value();
}

const mat4 & CameraEx::viewProjectionEx() const
{
    if (m_dirty)
        update();

    if (!m_viewProjection.isValid())
        m_viewProjection.setValue(projectionEx() * view());

    return m_viewProjection.value();
}

const mat4 & CameraEx::projectionInvertedEx() const
{
    if (m_dirty)
        update();

    if (!m_projectionInverted.isValid())
        m_projectionInverted.setValue(inverse(projectionEx()));

    return m_projectionInverted.value();
}

const mat4 & CameraEx::viewProjectionInvertedEx() const
{
    if (m_dirty)
        update();

    if (!m_viewProjectionInverted.isValid())
        m_viewProjectionInverted.setValue(inverse(viewProjectionEx()));

    return m_viewProjectionInverted.value();
}
