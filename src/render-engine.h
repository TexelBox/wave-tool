#ifndef WAVE_TOOL_RENDER_ENGINE_H_
#define WAVE_TOOL_RENDER_ENGINE_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "camera.h"
#include "mesh-object.h"
#include "shader-tools.h"
#include "texture.h"

#include "lodepng.h"

namespace wave_tool {
    class RenderEngine {
        public:
            RenderEngine(GLFWwindow *window, std::shared_ptr<Camera> camera);

            void render(std::vector<std::shared_ptr<MeshObject>> const& objects);
            void renderLight();
            void assignBuffers(MeshObject &object);
            void updateBuffers(MeshObject &object, bool const updateVerts, bool const updateUVs, bool const updateNormals, bool const updateColours);

            void setWindowSize(int width, int height);

            void updateLightPos(glm::vec3 add);

            unsigned int loadTexture(std::string filename);
        private:
            GLFWwindow *window = nullptr;
            std::shared_ptr<Camera> camera = nullptr;

            GLuint trivialProgram;
            GLuint mainProgram;
            GLuint lightProgram;

            glm::mat4 projection;
            glm::vec3 lightPos;
    };
}

#endif // WAVE_TOOL_RENDER_ENGINE_H_
