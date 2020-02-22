/*
 * Sample boilerplate taken from CPSC453 at the University of Calgary
 * Main class for user-defined program
 * Created on: Sep 10, 2018
 * Author: John Hall
 * Modifications by: Aaron Hornby (10176084)
 */

#include "program.h"

#include <fstream>
#include <iostream>
#include <string>

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_opengl3.h"

// remember: include glad before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace prefix {
    Program::Program() {}

    Program::~Program() {}

    bool Program::start() {
        if (!setupWindow()) return false;

        //image.Initialize();
        //do a bunch of raytracing into texture
        //image.SaveToFile("image.png"); // no need to put in loop since we dont update image

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        // render loop
        while (!glfwWindowShouldClose(m_window)) {
            // handle inputs
            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            buildUI();

            // rendering...
            //image.Render();
            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(m_window);
        }

        return cleanup();
    }

    void Program::buildUI() {
        // start Dear ImGui frame...
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(1024.0f, 64.0f), ImVec2(1024.0f, 512.0f));
        ImGui::Begin("SETTINGS");

        ImGui::Separator();

        ImGui::Text("TODO...");

        ImGui::Separator();

        ImGui::End();

        ImGui::EndFrame();
    }

    bool Program::cleanup() {
        // Dear ImGui cleanup...
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // glfw cleanup...
        if (nullptr != m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
        return true;
    }

    void Program::queryGLVersion() {
        // query OpenGL version and renderer information
        std::string const GLV = reinterpret_cast<char const*>(glGetString(GL_VERSION));
        std::string const GLSLV = reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        std::string const GLR = reinterpret_cast<char const*>(glGetString(GL_RENDERER));

        std::cout << "OpenGL [ " << GLV << " ] " << "with GLSL [ " << GLSLV << " ] " << "on renderer [ " << GLR << " ]" << std::endl;
    }

    bool Program::setupWindow() {
        // initialize the GLFW windowing system
        if (!glfwInit()) {
            std::cout << "ERROR: GLFW failed to initialize, TERMINATING..." << std::endl;
            return false;
        }

        // set the custom error callback function
        // errors will be printed to the console
        glfwSetErrorCallback(errorCallback);

        // attempt to create a window with an OpenGL 4.1 core profile context
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // reference: https://stackoverflow.com/questions/42848322/what-does-my-choice-of-glfw-samples-actually-do
        //glfwWindowHint(GLFW_SAMPLES, 4);
        //glEnable(GL_MULTISAMPLE);
        int const WIDTH = 1024;
        int const HEIGHT = 1024;
        m_window = glfwCreateWindow(WIDTH, HEIGHT, "WaveTool", nullptr, nullptr);
        if (!m_window) {
            std::cout << "ERROR: Program failed to create GLFW window, TERMINATING..." << std::endl;
            glfwTerminate();
            return false;
        }

        // so that we can access this program object on key callbacks...
        glfwSetWindowUserPointer(m_window, this);

        // set the custom function that tracks key presses
        glfwSetKeyCallback(m_window, keyCallback);

        // set callback for window resizing
        glfwSetWindowSizeCallback(m_window, windowSizeCallback);

        // bring the new window to the foreground (not strictly necessary but convenient)
        glfwMakeContextCurrent(m_window);
        // enable VSync
        glfwSwapInterval(1);

        // reference: https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library#glad_.28Multi-Language_GL.2FGLES.2FEGL.2FGLX.2FWGL_Loader-Generator.29
        // glad uses GLFW loader to find appropriate OpenGL config (load OpenGL functions) for your system
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << "ERROR: Failed to initialize OpenGL context, TERMINATING..." << std::endl;
            glfwTerminate();
            return false;
        }

        // reference: https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
        // setup Dear ImGui context...
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        // setup platform/renderer bindings...
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        char const* glsl_version = "#version 410 core";
        //NOTE: must init glad before this call to not get an exception
        // reference: https://stackoverflow.com/questions/48582444/imgui-with-the-glad-opengl-loader-throws-segmentation-fault-core-dumped
        ImGui_ImplOpenGL3_Init(glsl_version);
        // set UI style...
        ImGui::StyleColorsDark();

        // query and print out information about our OpenGL environment
        queryGLVersion();

        return true;
    }

    void errorCallback(int error, char const* description) {
        std::cout << "GLFW ERROR: " << error << ":" << std::endl;
        std::cout << description << std::endl;
    }

    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        Program *program = (Program*)glfwGetWindowUserPointer(window);

        // key codes are often prefixed with GLFW_KEY_ and can be found on the GLFW website
        if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    void windowSizeCallback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    }

}
