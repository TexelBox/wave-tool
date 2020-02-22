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

        // render loop
        while (!glfwWindowShouldClose(window)) {
            //image.Render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

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
        int const WIDTH = 1024;
        int const HEIGHT = 1024;
        window = glfwCreateWindow(WIDTH, HEIGHT, "WaveTool", NULL, NULL);
        if (!window) {
            std::cout << "ERROR: Program failed to create GLFW window, TERMINATING..." << std::endl;
            glfwTerminate();
            return false;
        }

        // so that we can access this program object on key callbacks...
        glfwSetWindowUserPointer(window, this);

        // set the custom function that tracks key presses
        glfwSetKeyCallback(window, keyCallback);

        // bring the new window to the foreground (not strictly necessary but convenient)
        glfwMakeContextCurrent(window);
        // enable VSync
        glfwSwapInterval(1);

        // reference: https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library#glad_.28Multi-Language_GL.2FGLES.2FEGL.2FGLX.2FWGL_Loader-Generator.29
        // glad uses GLFW loader to find appropriate OpenGL config (load OpenGL functions) for your system
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << "ERROR: Failed to initialize OpenGL context, TERMINATING..." << std::endl;
            glfwTerminate();
            return false;
        }

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
}
