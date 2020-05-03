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
#include <string>
#include <vector>

struct GLFWwindow;

namespace wave_tool {
    class Camera;
    class MeshObject;
    class RenderEngine;

    class Program {
        public:
            static unsigned int const s_IMAGE_SAVE_AS_NAME_CHAR_LIMIT{128};

            Program();
            ~Program();

            std::shared_ptr<RenderEngine> getRenderEngine() const;

            // runs the user defined program (including render loop)
            bool start();
        private:
            char m_imageSaveAsName[s_IMAGE_SAVE_AS_NAME_CHAR_LIMIT]{"image"};
            std::vector<std::shared_ptr<MeshObject>> m_meshObjects;
            std::shared_ptr<RenderEngine> m_renderEngine = nullptr;
            std::shared_ptr<MeshObject> m_skyboxClouds = nullptr;
            std::shared_ptr<MeshObject> m_skyboxStars = nullptr;
            std::shared_ptr<MeshObject> m_skysphere = nullptr;
            std::shared_ptr<MeshObject> m_terrain = nullptr;
            std::shared_ptr<MeshObject> m_waterGrid = nullptr;
            GLFWwindow *m_window = nullptr;
            std::shared_ptr<MeshObject> m_xyPlane = nullptr;
            std::shared_ptr<MeshObject> m_xzPlane = nullptr;
            std::shared_ptr<MeshObject> m_yzPlane = nullptr;

            // constructs Dear ImGui UI components
            void buildUI();
            bool cleanup();
            void exportFrontBufferToImageFile(std::string const& filePath);
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
