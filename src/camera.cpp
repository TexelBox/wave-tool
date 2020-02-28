// reference: https://learnopengl.com/Getting-started/Camera

#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace wave_tool {
    // init statics...
    glm::vec3 const Camera::DEFAULT_FORWARD = glm::vec3(0.0f, 0.0f, -1.0f);
    float const Camera::DEFAULT_PITCH = 0.0f;
    glm::vec3 const Camera::DEFAULT_POSITION = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 const Camera::DEFAULT_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 const Camera::DEFAULT_UP = glm::vec3(0.0f, 1.0f, 0.0f);
    float const Camera::DEFAULT_YAW = -90.0f;

    Camera::Camera(float const fov, float const aspect, float const nearClipDistance, float const farClipDistance, glm::vec3 const& position, glm::vec3 const& forward, glm::vec3 const& up, float const yaw, float const pitch)
        : m_fov(fov), m_aspect(aspect), m_nearClipDistance(nearClipDistance), m_farClipDistance(farClipDistance), m_position(position), m_forward(forward), m_up(up), m_yaw(yaw), m_pitch(pitch)
    {
        m_upInit = m_up;
        m_right = glm::cross(m_forward, m_up);
        updateBasisVectors();
        updateProjectionMat();
    }

    glm::mat4 Camera::getProjectionMat() const {
        return m_projectionMat;
    }

    glm::mat4 Camera::getViewMat() const {
        return m_viewMat;
    }

    void Camera::rotate(float const deltaYawDegrees, float const deltaPitchDegrees) {
        m_yaw = glm::mod(m_yaw + deltaYawDegrees, 360.0f);
        m_pitch = glm::clamp(m_pitch + deltaPitchDegrees, -89.0f, 89.0f);

        updateBasisVectors();
    }

    void Camera::setAspect(float const aspect) {
        m_aspect = aspect;

        updateProjectionMat();
    }

    void Camera::translate(glm::vec3 const& deltaPosition) {
        m_position += deltaPosition;

        updateViewMat();
    }

    void Camera::translateForward(float const delta) {
        m_position += m_forward * delta;

        updateViewMat();
    }

    void Camera::translateRight(float const delta) {
        m_position += m_right * delta;

        updateViewMat();
    }

    void Camera::translateUp(float const delta) {
        m_position += m_up * delta;

        updateViewMat();
    }

    void Camera::zoom(float const deltaFOV) {
        m_fov = glm::clamp(m_fov + deltaFOV, 15.0f, 105.0f);

        updateProjectionMat();
    }

    void Camera::updateBasisVectors() {
        m_forward = glm::normalize(glm::vec3(glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch)),
                                             glm::sin(glm::radians(m_pitch)),
                                             glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch))));
        m_right = glm::normalize(glm::cross(m_forward, m_upInit));
        m_up = glm::normalize(glm::cross(m_right, m_forward));

        updateViewMat();
    }

    void Camera::updateProjectionMat() {
        m_projectionMat = glm::perspective(glm::radians(m_fov), m_aspect, m_nearClipDistance, m_farClipDistance);
    }

    void Camera::updateViewMat() {
        m_viewMat = glm::lookAt(m_position, m_position + m_forward, m_up);
    }
}
