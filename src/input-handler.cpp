#include "input-handler.h"
#include "program.h"

namespace wave_tool {
    std::shared_ptr<RenderEngine> InputHandler::renderEngine = nullptr;
    std::shared_ptr<Camera> InputHandler::camera = nullptr;
    int InputHandler::mouseOldX;
    int InputHandler::mouseOldY;

    // Must be called before processing any GLFW events
    void InputHandler::setUp(std::shared_ptr<RenderEngine> renderEngine, std::shared_ptr<Camera> camera) {
        InputHandler::renderEngine = renderEngine;
        InputHandler::camera = camera;
    }

    // Callback for key presses
    void InputHandler::key(GLFWwindow *window, int key, int scancode, int action, int mods) {

        Program *program = (Program*)glfwGetWindowUserPointer(window);

        if (GLFW_PRESS == action || GLFW_REPEAT == action) { // key press, or press & hold
            if (GLFW_KEY_ESCAPE == key) glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    // Callback for mouse button presses
    void InputHandler::mouse(GLFWwindow *window, int button, int action, int mods) {

        Program *program = (Program*)glfwGetWindowUserPointer(window);

        if (action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            mouseOldX = x;
            mouseOldY = y;
        }
    }

    // Callback for mouse motion
    void InputHandler::motion(GLFWwindow *window, double x, double y) {
        double dx, dy;
        dx = (x - mouseOldX);
        dy = (y - mouseOldY);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
            camera->updateLongitudeRotation(dx * 0.5);
            camera->updateLatitudeRotation(dy * 0.5);
        }

        mouseOldX = x;
        mouseOldY = y;
    }

    // Callback for mouse scroll
    void InputHandler::scroll(GLFWwindow *window, double x, double y) {
        double dy;
        dy = (x - y);
        //camera->updatePosition(glm::vec3(0.0, 0.0, dy * 0.1));
        camera->updatePosition(glm::vec3(0.0, 0.0, dy * 10)); // faster scrolling
    }

    // Callback for window reshape/resize
    void InputHandler::reshape(GLFWwindow *window, int width, int height) {
        renderEngine->setWindowSize(width, height);
    }
}
