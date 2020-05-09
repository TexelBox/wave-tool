// BSD 3 - Clause License
//
// Copyright(c) 2020, Aaron Hornby
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//     OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

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

    float Camera::getAspect() const { return m_aspect; }

    float Camera::getFarClipDistance() const { return m_farClipDistance; }

    glm::vec3 Camera::getForward() const { return m_forward; }

    float Camera::getFOV() const { return m_fov; }

    float Camera::getNearClipDistance() const { return m_nearClipDistance; }

    float Camera::getPitch() const { return m_pitch; }

    glm::vec3 Camera::getPosition() const { return m_position; }

    glm::mat4 Camera::getProjectionMat() const { return m_projectionMat; }

    glm::vec3 Camera::getRight() const { return m_right; }

    glm::vec3 Camera::getUp() const { return m_up; }

    glm::mat4 Camera::getViewMat() const { return m_viewMat; }

    float Camera::getYaw() const { return m_yaw; }

    void Camera::rotate(float const deltaYawDegrees, float const deltaPitchDegrees) {
        m_yaw = glm::mod(m_yaw + deltaYawDegrees, 360.0f);
        m_pitch = glm::clamp(m_pitch + deltaPitchDegrees, -89.0f, 89.0f);

        updateBasisVectors();
    }

    void Camera::setAspect(float const aspect) {
        m_aspect = aspect;

        updateProjectionMat();
    }

    void Camera::setRotation(float const yawDegrees, float const pitchDegrees) {
        m_yaw = glm::mod(yawDegrees, 360.0f);
        m_pitch = glm::clamp(pitchDegrees, -89.0f, 89.0f);

        updateBasisVectors();
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
