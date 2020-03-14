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

#include <imgui/imgui.h>
#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_opengl3.h>

// remember: include glad before GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "input-handler.h"
#include "mesh-object.h"
#include "object-loader.h"
#include "render-engine.h"

namespace wave_tool {
    Program::Program() {}

    Program::~Program() {}

    std::shared_ptr<RenderEngine> Program::getRenderEngine() const {
        return m_renderEngine;
    }

    bool Program::start() {
        if (!setupWindow()) return false;

        m_renderEngine = std::make_shared<RenderEngine>(m_window);

        initScene();

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
            ImGui::Render();
            //image.Render();
            m_renderEngine->render(m_skybox, m_meshObjects);

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
        //ImGui::SetWindowSize(ImVec2(1024.0f, 256.0f));
        ImGui::Begin("SETTINGS");

        ImGui::Separator();

        ImGui::Text("TODO...");

        ImGui::Separator();

        if (nullptr != m_yzPlane) {
            if (ImGui::Button("TOGGLE YZ-PLANE (RED)")) m_yzPlane->m_isVisible = !m_yzPlane->m_isVisible;
            ImGui::SameLine();
        }
        if (nullptr != m_xzPlane) {
            if (ImGui::Button("TOGGLE XZ-PLANE (GREEN)")) m_xzPlane->m_isVisible = !m_xzPlane->m_isVisible;
            ImGui::SameLine();
        }
        if (nullptr != m_xyPlane) {
            if (ImGui::Button("TOGGLE XY-PLANE (BLUE)")) m_xyPlane->m_isVisible = !m_xyPlane->m_isVisible;
            ImGui::SameLine();
        }
        if (nullptr != m_yzPlane || nullptr != m_xzPlane || nullptr != m_xyPlane) {
            ImGui::Text("   note: grid spacing is 10 units");
            ImGui::Separator();
        }

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

    //NOTE: this method should only be called ONCE at start
    void Program::initScene() {
        // CREATE THE 3 PLANES...

        // draw a symmetrical grid for each cartesian plane...

        //NOTE: compare this to far clipping plane distance of 5000
        //NOTE: all these should be the same
        int const maxX = 2500;
        int const maxY = 2500;
        int const maxZ = 2500;
        //NOTE: any change here should be reflected in the ImGui notice
        int const deltaX = 10;
        int const deltaY = 10;
        int const deltaZ = 10;

        // YZ PLANE

        m_yzPlane = std::make_shared<MeshObject>();

        for (int y = -maxY; y <= maxY; y += deltaY) {
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, y, -maxZ));
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, y, maxZ));
        }
        for (int z = -maxZ; z <= maxZ; z += deltaZ) {
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, -maxY, z));
            m_yzPlane->drawVerts.push_back(glm::vec3(0.0f, maxY, z));
        }

        for (unsigned int i = 0; i < m_yzPlane->drawVerts.size(); ++i) {
            m_yzPlane->drawFaces.push_back(i);
            m_yzPlane->colours.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        }

        m_yzPlane->m_primitiveMode = PrimitiveMode::LINES;
        m_meshObjects.push_back(m_yzPlane);
        m_renderEngine->assignBuffers(*m_yzPlane);

        // XZ PLANE

        m_xzPlane = std::make_shared<MeshObject>();

        for (int x = -maxX; x <= maxX; x += deltaX) {
            m_xzPlane->drawVerts.push_back(glm::vec3(x, 0.0f, -maxZ));
            m_xzPlane->drawVerts.push_back(glm::vec3(x, 0.0f, maxZ));
        }
        for (int z = -maxZ; z <= maxZ; z += deltaZ) {
            m_xzPlane->drawVerts.push_back(glm::vec3(-maxX, 0.0f, z));
            m_xzPlane->drawVerts.push_back(glm::vec3(maxX, 0.0f, z));
        }

        for (unsigned int i = 0; i < m_xzPlane->drawVerts.size(); ++i) {
            m_xzPlane->drawFaces.push_back(i);
            m_xzPlane->colours.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        }

        m_xzPlane->m_primitiveMode = PrimitiveMode::LINES;
        m_meshObjects.push_back(m_xzPlane);
        m_renderEngine->assignBuffers(*m_xzPlane);

        // XY PLANE

        m_xyPlane = std::make_shared<MeshObject>();

        for (int x = -maxX; x <= maxX; x += deltaX) {
            m_xyPlane->drawVerts.push_back(glm::vec3(x, -maxY, 0.0f));
            m_xyPlane->drawVerts.push_back(glm::vec3(x, maxY, 0.0f));
        }
        for (int y = -maxY; y <= maxY; y += deltaY) {
            m_xyPlane->drawVerts.push_back(glm::vec3(-maxX, y, 0.0f));
            m_xyPlane->drawVerts.push_back(glm::vec3(maxX, y, 0.0f));
        }

        for (unsigned int i = 0; i < m_xyPlane->drawVerts.size(); ++i) {
            m_xyPlane->drawFaces.push_back(i);
            m_xyPlane->colours.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
        }

        m_xyPlane->m_primitiveMode = PrimitiveMode::LINES;
        m_meshObjects.push_back(m_xyPlane);
        m_renderEngine->assignBuffers(*m_xyPlane);

        // yoshi placeholder...
        /*
        std::shared_ptr<MeshObject> yoshi = ObjectLoader::createTriMeshObject("../assets/models/imports/yoshi.obj", false, true);
        if (nullptr != yoshi) {
            if (yoshi->hasTexture) {
                yoshi->textureID = m_renderEngine->load2DTexture("../assets/textures/yoshi.png");
                // if there was an error...
                if (0 == yoshi->textureID) yoshi->textureID = m_renderEngine->load2DTexture("../assets/textures/default.png"); // fallback#1 (if this fails too for some reason, then the model will most likely be black or undefined colour)
            }
            yoshi->generateNormals();
            m_meshObjects.push_back(yoshi);
            m_renderEngine->assignBuffers(*yoshi);
        }
        */

        //TODO: save a bunch of skyboxes that can be toggled back and forth (along with sun position?)
        // hard-coded skybox...
        m_skybox = ObjectLoader::createTriMeshObject("../../assets/models/imports/cube.obj", true, true);
        if (nullptr != m_skybox) {
            m_skybox->textureID = m_renderEngine->loadCubemap({"../../assets/textures/skyboxes/sunny/TropicalSunnyDay_px.jpg",
                                                               "../../assets/textures/skyboxes/sunny/TropicalSunnyDay_nx.jpg",
                                                               "../../assets/textures/skyboxes/sunny/TropicalSunnyDay_py.jpg",
                                                               "../../assets/textures/skyboxes/sunny/TropicalSunnyDay_ny.jpg",
                                                               "../../assets/textures/skyboxes/sunny/TropicalSunnyDay_pz.jpg",
                                                               "../../assets/textures/skyboxes/sunny/TropicalSunnyDay_nz.jpg"});
            // if there was an error
            // fallback#1 (use debug skybox) (if this fails too for some reason, then there won't be a skybox)
            if (0 == m_skybox->textureID) m_skybox->textureID = m_renderEngine->loadCubemap({"../../assets/textures/skyboxes/debug/_px.jpg",
                                                                                             "../../assets/textures/skyboxes/debug/_nx.jpg",
                                                                                             "../../assets/textures/skyboxes/debug/_py.jpg",
                                                                                             "../../assets/textures/skyboxes/debug/_ny.jpg",
                                                                                             "../../assets/textures/skyboxes/debug/_pz.jpg",
                                                                                             "../../assets/textures/skyboxes/debug/_nz.jpg"});
            // fallback#2 (no skybox, you will just see the clear colour)
            if (0 == m_skybox->textureID) m_skybox = nullptr;
        }
        m_renderEngine->assignBuffers(*m_skybox);

        //TODO: in the future, allow users to load in different terrains? (it would be nice to get program to work dynamically with whatever terrain it comes across) - probably not since finding terrain that works with my loader is hell
        // terrain...
        m_terrain = ObjectLoader::createTriMeshObject("../../assets/models/imports/everest.obj");
        if (nullptr != m_terrain) {
            if (m_terrain->hasTexture) {
                m_terrain->textureID = m_renderEngine->load2DTexture("../../assets/textures/everest.png");
                // if there was an error...
                if (0 == m_terrain->textureID) m_terrain->textureID = m_renderEngine->load2DTexture("../../assets/textures/default.png"); // fallback#1 (if this fails too for some reason, then the model will most likely be black or undefined colour)
            }
            //m_terrain->generateNormals();
            m_terrain->setScale(glm::vec3(1000.0f, 1000.0f, 1000.0f));
            m_meshObjects.push_back(m_terrain);
            m_renderEngine->assignBuffers(*m_terrain);
        }
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

        // so that we can access this program object in callbacks...
        glfwSetWindowUserPointer(m_window, this);

        // set callbacks...
        glfwSetCursorPosCallback(m_window, InputHandler::motion);
        //glfwSetKeyCallback(m_window, keyCallback);
        glfwSetKeyCallback(m_window, InputHandler::key);
        glfwSetMouseButtonCallback(m_window, InputHandler::mouse);
        glfwSetScrollCallback(m_window, InputHandler::scroll);
        //glfwSetWindowSizeCallback(m_window, windowSizeCallback);
        glfwSetWindowSizeCallback(m_window, InputHandler::reshape);

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

/*
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        Program *program = (Program*)glfwGetWindowUserPointer(window);

        // key codes are often prefixed with GLFW_KEY_ and can be found on the GLFW website
        if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }
*/

/*
    void windowSizeCallback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    }
*/
}
