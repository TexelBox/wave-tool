/*
 * Sample boilerplate taken from CPSC453 at the University of Calgary
 * Main class for user-defined program
 * Created on: Sep 10, 2018
 * Author: John Hall
 * Modifications by: Aaron Hornby (10176084)
 */

#ifndef WAVE_TOOL_PROGRAM_H_
#define WAVE_TOOL_PROGRAM_H_

#include <memory>
#include <vector>

//#include "image-buffer.h"

struct GLFWwindow;

namespace wave_tool {
    class Camera;
    class MeshObject;
    class RenderEngine;

    class Program {
        public:
            Program();
            ~Program();

            std::shared_ptr<RenderEngine> getRenderEngine() const;

            // runs the user defined program (including render loop)
            bool start();
        private:
            //ImageBuffer image;
            std::vector<std::shared_ptr<MeshObject>> m_meshObjects;
            std::shared_ptr<RenderEngine> m_renderEngine = nullptr;
            std::shared_ptr<MeshObject> m_skybox = nullptr;
            std::shared_ptr<MeshObject> m_terrain = nullptr;
            std::shared_ptr<MeshObject> m_waterGrid = nullptr;
            GLFWwindow *m_window = nullptr;
            std::shared_ptr<MeshObject> m_xyPlane = nullptr;
            std::shared_ptr<MeshObject> m_xzPlane = nullptr;
            std::shared_ptr<MeshObject> m_yzPlane = nullptr;

            // constructs Dear ImGui UI components
            void buildUI();
            bool cleanup();
            void initScene();
            // prints system specs to the console
            void queryGLVersion();
            // initializes GLFW and creates the window
            bool setupWindow();
    };

    // functions passed to GLFW to handle errors and keyboard input
    //NOTE: GLFW requires them to not be member functions of a class
    void errorCallback(int error, char const* description);
    //void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    //void windowSizeCallback(GLFWwindow *window, int width, int height);
}

#endif // WAVE_TOOL_PROGRAM_H_
