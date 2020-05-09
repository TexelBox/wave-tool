#ifndef WAVE_TOOL_PROGRAM_H_
#define WAVE_TOOL_PROGRAM_H_

// BSD 3 - Clause License
//
// Copyright(c) 2020, Aaron Hornby
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//     OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

/*
 * Sample boilerplate taken from CPSC453 at the University of Calgary
 * Main class for user-defined program
 * Created on: Sep 10, 2018
 * Author: John Hall
 * Modifications by: Aaron Hornby (10176084)
 */

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
