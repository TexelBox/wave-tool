#include "input-handler.h"

#include <imgui/imgui.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "render-engine.h"
#include "program.h"

//NOTE: most callbacks are guarded in order for our main application to ignore inputs when user is interacting with UI
// reference: https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-to-my-application
namespace wave_tool {
    // init statics...
    int InputHandler::mouseOldX = 0;
    int InputHandler::mouseOldY = 0;

    // Callback for key presses
    void InputHandler::key(GLFWwindow *window, int key, int scancode, int action, int mods) {
        //TODO: I may want to also check io.WantCaptureMouse (or some window hovering check), or probably best to just individually guard special keys like CTRL (i.e. to handle CTRL+LEFT_CLICK UI inputs)
        ImGuiIO const& io{ImGui::GetIO()};
        if (io.WantCaptureKeyboard) return;

        Program *program = (Program*)glfwGetWindowUserPointer(window);

        if (GLFW_PRESS == action || GLFW_REPEAT == action) { // key press, or press & hold

            //TODO: multiply by frameTime
            float const CAMERA_SPEED{0.1f};

            switch (key) {
                case GLFW_KEY_A:
                    program->getRenderEngine()->getCamera()->translateRight(-CAMERA_SPEED);
                    break;
                case GLFW_KEY_D:
                    program->getRenderEngine()->getCamera()->translateRight(CAMERA_SPEED);
                    break;
                case GLFW_KEY_E:
                    program->getRenderEngine()->getCamera()->translateUp(CAMERA_SPEED);
                    break;
                case GLFW_KEY_Q:
                    program->getRenderEngine()->getCamera()->translateUp(-CAMERA_SPEED);
                    break;
                case GLFW_KEY_S:
                    program->getRenderEngine()->getCamera()->translateForward(-CAMERA_SPEED);
                    break;
                case GLFW_KEY_W:
                    program->getRenderEngine()->getCamera()->translateForward(CAMERA_SPEED);
                    break;
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GL_TRUE);
                    break;
            }
        }
    }

    // Callback for mouse button presses
    void InputHandler::mouse(GLFWwindow *window, int button, int action, int mods) {
        ImGuiIO const& io{ImGui::GetIO()};
        if (io.WantCaptureMouse) return;

        Program *program = (Program*)glfwGetWindowUserPointer(window);

        if (GLFW_PRESS == action) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            mouseOldX = x;
            mouseOldY = y;
        }
    }

    // Callback for mouse motion
    void InputHandler::motion(GLFWwindow *window, double x, double y) {
        ImGuiIO const& io{ImGui::GetIO()};
        if (io.WantCaptureMouse) return;

        Program *program = (Program*)glfwGetWindowUserPointer(window);

        if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
            double dx, dy;
            dx = x - mouseOldX;
            dy = y - mouseOldY;

            //TODO: multiply by frameTime
            float const CAMERA_SENSITIVITY = 0.5f;

            //NOTE: dy negated due to y-coords ranging from bottom to top
            program->getRenderEngine()->getCamera()->rotate(dx * CAMERA_SENSITIVITY, -dy * CAMERA_SENSITIVITY);
        }

        mouseOldX = x;
        mouseOldY = y;
    }

    // Callback for mouse scroll
    void InputHandler::scroll(GLFWwindow *window, double x, double y) {
        ImGuiIO const& io{ImGui::GetIO()};
        if (io.WantCaptureMouse) return;

        Program *program = (Program*)glfwGetWindowUserPointer(window);

        double dy;
        dy = x - y;

        //TODO: multiply by frameTime
        float const CAMERA_ZOOM_SENSITIVITY = 10.0f;

        program->getRenderEngine()->getCamera()->zoom(dy * CAMERA_ZOOM_SENSITIVITY);
    }

    // Callback for window reshape/resize
    void InputHandler::reshape(GLFWwindow *window, int width, int height) {

        Program *program = (Program*)glfwGetWindowUserPointer(window);

        program->getRenderEngine()->setWindowSize(width, height);
    }
}
