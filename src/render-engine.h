#ifndef WAVE_TOOL_RENDER_ENGINE_H_
#define WAVE_TOOL_RENDER_ENGINE_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <vector>

#include "camera.h"
#include "mesh-object.h"
#include "shader-tools.h"
#include "texture.h"

namespace wave_tool {
    class RenderEngine {
        public:
            RenderEngine(GLFWwindow *window);

            std::shared_ptr<Camera> getCamera() const;

            void render(std::shared_ptr<const MeshObject> skybox, std::vector<std::shared_ptr<MeshObject>> const& objects);
            //void renderLight();
            void assignBuffers(MeshObject &object);
            void updateBuffers(MeshObject &object, bool const updateVerts, bool const updateUVs, bool const updateNormals, bool const updateColours);

            void setWindowSize(int width, int height);

            void updateLightPos(glm::vec3 add);

            GLuint load2DTexture(std::string const& filePath);
            GLuint loadCubemap(std::vector<std::string> const& faces);
        private:
            std::shared_ptr<Camera> m_camera = nullptr;

            GLuint skyboxProgram;
            GLuint trivialProgram;
            GLuint mainProgram;
            //GLuint lightProgram;

            glm::vec3 lightPos;
    };
}

#endif // WAVE_TOOL_RENDER_ENGINE_H_
