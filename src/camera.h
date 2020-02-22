#ifndef WAVE_TOOL_CAMERA_H_
#define WAVE_TOOL_CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace wave_tool {
    class Camera {
        public:
            Camera();
            virtual ~Camera();

            glm::mat4 getLookAt();
            glm::vec3 getPosition();
            void updateLongitudeRotation(float rad);
            void updateLatitudeRotation(float rad);
            void updatePosition(glm::vec3 value);

        private:
            glm::vec3 eye;
            glm::vec3 up;
            glm::vec3 centre;

            float longitudeRotRad;
            float latitudeRotRad;
    };
}

#endif // WAVE_TOOL_CAMERA_H_
