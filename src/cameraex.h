#pragma once

#include <glowutils/Camera.h>

class CameraEx : public glowutils::Camera
{
public:
    CameraEx(
        const glm::vec3 & eye = glm::vec3(0.0, 0.0, 1.0)
        , const glm::vec3 & center = glm::vec3(0.0, 0.0, 0.0)
        , const glm::vec3 & up = glm::vec3(0.0, 1.0, 0.0));

    float left() const;
    void setLeft(float left);
    float right() const;
    void setRight(float right);
    float bottom() const;
    void setBottom(float bottom);
    float top() const;
    void setTop(float top);

    // hack this, because virtuality lack in glowutils::Camera...
    /** zNear used for orthographic projection */
    float zNearOrtho() const;
    /** set the zNear used for orthographic projection */
    void setZNearOtho(float zNear);

    // lazy matrices getters

    /** projection() is not virtual in glowutils::Camera, that's why we have to hack this */
    const glm::mat4 & projectionOrthographic() const;
    const glm::mat4 & viewProjectionOrthographic() const;
    const glm::mat4 & projectionOrthographicInverted() const;
    const glm::mat4 & viewProjectionOrthographicInverted() const;

    virtual void changed() const override;

protected:
    float m_zNearOrtho;
    float m_left;
    float m_right;
    float m_bottom;
    float m_top;

    glowutils::CachedValue<glm::mat4> m_projectionOrthographic;
    glowutils::CachedValue<glm::mat4> m_projectionOrthographicInverted;
    glowutils::CachedValue<glm::mat4> m_viewProjectionOrthographic;
    glowutils::CachedValue<glm::mat4> m_viewProjectionOrthographicInverted;
};