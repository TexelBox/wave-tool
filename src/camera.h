#ifndef WAVE_TOOL_CAMERA_H_
#define WAVE_TOOL_CAMERA_H_

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

#include <glm/glm.hpp>

namespace wave_tool {
    class Camera {
        public:
            static glm::vec3 const DEFAULT_FORWARD;
            static float const DEFAULT_PITCH;
            static glm::vec3 const DEFAULT_POSITION;
            static glm::vec3 const DEFAULT_RIGHT;
            static glm::vec3 const DEFAULT_UP;
            static float const DEFAULT_YAW;

            Camera(float const fov, float const aspect, float const nearClipDistance, float const farClipDistance, glm::vec3 const& position = DEFAULT_POSITION, glm::vec3 const& forward = DEFAULT_FORWARD, glm::vec3 const& up = DEFAULT_UP, float const yaw = DEFAULT_YAW, float const pitch = DEFAULT_PITCH);

            float getAspect() const;
            float getFarClipDistance() const;
            glm::vec3 getForward() const;
            float getFOV() const;
            float getNearClipDistance() const;
            float getPitch() const;
            glm::vec3 getPosition() const;
            glm::mat4 getProjectionMat() const;
            glm::vec3 getRight() const;
            glm::vec3 getUp() const;
            glm::mat4 getViewMat() const;
            float getYaw() const;
            void rotate(float const deltaYawDegrees, float const deltaPitchDegrees);
            void setAspect(float const aspect);
            void setRotation(float const yawDegrees, float const pitchDegrees);
            void translate(glm::vec3 const& deltaPosition);
            void translateForward(float const delta);
            void translateRight(float const delta);
            void translateUp(float const delta);
            void zoom(float const deltaFOV);
        private:
            float m_aspect;
            float m_farClipDistance;
            glm::vec3 m_forward;
            float m_fov;
            float m_nearClipDistance;
            float m_pitch;
            glm::vec3 m_position;
            glm::mat4 m_projectionMat;
            glm::vec3 m_right;
            glm::vec3 m_up;
            glm::vec3 m_upInit;
            glm::mat4 m_viewMat;
            float m_yaw;

            void updateBasisVectors();
            void updateProjectionMat();
            void updateViewMat();
    };
}

#endif // WAVE_TOOL_CAMERA_H_
