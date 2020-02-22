#ifndef WAVE_TOOL_INPUT_HANDLER_H_
#define WAVE_TOOL_INPUT_HANDLER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "camera.h"
#include "render-engine.h"

namespace wave_tool {
    class InputHandler {
        public:
            static void setUp(std::shared_ptr<RenderEngine> renderEngine, std::shared_ptr<Camera> camera);

            static void key(GLFWwindow *window, int key, int scancode, int action, int mods);
            static void mouse(GLFWwindow *window, int button, int action, int mods);
            static void motion(GLFWwindow *window, double x, double y);
            static void scroll(GLFWwindow *window, double x, double y);
            static void reshape(GLFWwindow *window, int width, int height);

        private:
            static std::shared_ptr<RenderEngine> renderEngine;
            static std::shared_ptr<Camera> camera;

            static int mouseOldX;
            static int mouseOldY;
    };
}

#endif // WAVE_TOOL_INPUT_HANDLER_H_
