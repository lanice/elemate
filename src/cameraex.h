#pragma once

#include <glowutils/Camera.h>

enum class ProjectionType {
    perspective,
    orthographic
};

class CameraEx : public glowutils::Camera
{
public:
    CameraEx(ProjectionType projection,
        const glm::vec3 & eye = glm::vec3(0.0, 0.0, 1.0)
        , const glm::vec3 & center = glm::vec3(0.0, 0.0, 0.0)
        , const glm::vec3 & up = glm::vec3(0.0, 1.0, 0.0));

    ProjectionType projectionType() const;
    void setProjectionType(ProjectionType projectionType);

    float left() const;
    void setLeft(float left);
    float right() const;
    void setRight(float right);
    float bottom() const;
    void setBottom(float bottom);
    float top() const;
    void setTop(float top);

    // hack this, because virtuality lack in glowutils::Camera...
    float zNearEx() const;
    void setZNearEx(float zNear);
    const glm::mat4 & projectionEx() const;
    const glm::mat4 & viewProjectionEx() const;
    const glm::mat4 & projectionInvertedEx() const;
    const glm::mat4 & viewProjectionInvertedEx() const;

    /** remove the nonvirtual functions that we can not configure to support orthographic projections */
    const glm::mat4 & projection() const = delete;
    const glm::mat4 & viewProjection() const = delete;
    const glm::mat4 & viewInverted() const = delete;
    const glm::mat4 & projectionInverted() const = delete;
    const glm::mat4 & viewProjectionInverted() const = delete;
    float zNear() const = delete;
    void setZNear(float zNear) = delete;

protected:
    ProjectionType m_projectionType;
    float m_left;
    float m_right;
    float m_bottom;
    float m_top;

    virtual void changed() const override;
};