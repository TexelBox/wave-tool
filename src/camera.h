#ifndef WAVE_TOOL_CAMERA_H_
#define WAVE_TOOL_CAMERA_H_

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
            glm::mat4 getProjectionMat() const;
            glm::mat4 getViewMat() const;
            void rotate(float const deltaYawDegrees, float const deltaPitchDegrees);
            void setAspect(float const aspect);
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
