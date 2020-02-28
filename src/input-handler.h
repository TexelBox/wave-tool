#ifndef WAVE_TOOL_INPUT_HANDLER_H_
#define WAVE_TOOL_INPUT_HANDLER_H_

struct GLFWwindow;

namespace wave_tool {
    class InputHandler {
        public:
            static void key(GLFWwindow *window, int key, int scancode, int action, int mods);
            static void mouse(GLFWwindow *window, int button, int action, int mods);
            static void motion(GLFWwindow *window, double x, double y);
            static void scroll(GLFWwindow *window, double x, double y);
            static void reshape(GLFWwindow *window, int width, int height);
        private:
            static int mouseOldX;
            static int mouseOldY;
    };
}

#endif // WAVE_TOOL_INPUT_HANDLER_H_
