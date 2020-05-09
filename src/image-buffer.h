#ifndef WAVE_TOOL_IMAGE_BUFFER_H_
#define WAVE_TOOL_IMAGE_BUFFER_H_

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

// ==========================================================================
// Pixel Rendering and Image Saving Support Code
//  - requires the OpenGL Mathmematics (GLM) library: http://glm.g-truc.net
//  - requires the Magick++ development libraries: http://www.imagemagick.org
//  -   or the FreeImage library: http://freeimage.sourceforge.net
//  -   or the STB image write library: https://github.com/nothings/stb
//
// You may use this code (or not) however you see fit for your work.
//
// Authors: Sonny Chan, Alex Brown
//          University of Calgary
// Date:    2016-2018
// Modifications by: Aaron Hornby (10176084)
// ==========================================================================

#include <string>
#include <vector>

#include <glm/vec3.hpp>
#define NOMINMAX
//**Must include glad and GLFW in this order or it breaks**
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// --------------------------------------------------------------------------
// This class encapsulates functionality for setting pixel colours in an
// image memory buffer, copying the buffer into an OpenGL window for display,
// and saving the buffer to disk as an image file.

namespace wave_tool {
    class ImageBuffer {
        public:
            ImageBuffer();
            ~ImageBuffer();

            // returns the width or height of the currently allocated image
            int Width() const  { return m_width; }
            int Height() const { return m_height; }

            // call this after your OpenGL context is all set up to create an image
            // buffer that matches the size of your viewport
            bool Initialize();

            // call this if you need to delete the framebuffer object and texture
            void Destroy();

            // set a pixel in this image buffer to a specified colour:
            //  - (0,0) is the bottom-left pixel of the image
            //  - colour is RGB given as floating point numbers in the range [0,1]
            void SetPixel(int x, int y, glm::vec3 colour);

            void readFromFrontBuffer();

            // I need to call SetPixel from [0,0] to [1023,1023]

            // call this in your render function to copy this image onto your screen
            void Render();

            // call this at the end of your render to save the image to file
            bool SaveToFile(std::string const& imageFileName);
        private:
            // OpenGL texture corresponding to our image, and an FBO to render it
            GLuint  m_textureName;
            GLuint  m_framebufferObject;

            // dimensions of our image, and the pixel colour data array
            int     m_width, m_height;
            std::vector<glm::vec3> m_imageData; // vector of RGB tuples

            // state variables to keep track of modified region
            bool    m_modified;
            int     m_modifiedLower, m_modifiedUpper;

            void ResetModified();
    };
}

// --------------------------------------------------------------------------
#endif // WAVE_TOOL_IMAGE_BUFFER_H_
