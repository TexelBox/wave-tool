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
// Set the #defines below to choose the image library you have installed on
// your system, that you will be using for your assignment. Then compile and
// link this source file with your project.
//
// Authors: Sonny Chan, Alex Brown
//          University of Calgary
// Date:    2016-2018
// Modifications by: Aaron Hornby (10176084)
// ==========================================================================

#include "image-buffer.h"

#include <algorithm>
#include <iostream>

#include <glm/common.hpp>

// --------------------------------------------------------------------------
// Set these defines to choose which image library to use for saving image
// files to disk. Obviously, you shouldn't set more than one!

#define USE_STB_IMAGE
//#define USE_IMAGEMAGICK
//#define USE_FREEIMAGE

#ifdef USE_STB_IMAGE
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#endif
#ifdef USE_IMAGEMAGICK
#include <Magick++.h>
#endif
#ifdef USE_FREEIMAGE
#include <FreeImage.h>
#endif

//using namespace std;
//using namespace glm;

// --------------------------------------------------------------------------

//TODO: refactor this class to work with RGBA (32-bit) images
namespace wave_tool {
    ImageBuffer::ImageBuffer()
        : m_textureName(0), m_framebufferObject(0),
        m_width(0), m_height(0), m_modified(false) {}

    ImageBuffer::~ImageBuffer() {
        Destroy();
    }

    void ImageBuffer::ResetModified() {
        m_modified = false;
        m_modifiedLower = m_height;
        m_modifiedUpper = 0;
    }

    // --------------------------------------------------------------------------

    bool ImageBuffer::Initialize() {
        // retrieve the current viewport size
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        m_width = viewport[2];
        m_height = viewport[3];

        // allocate image data
        m_imageData.resize(m_width * m_height);
        for (int i = 0, k = 0; i < m_height; ++i) {
            for (int j = 0; j < m_width; ++j, ++k) {
                int p = (i >> 4) + (j >> 4);
                float c = 0.2 + ((p & 1) ? 0.1f : 0.0f);
                m_imageData[k] = glm::vec3(c);
            }
        }

        // allocate texture object
        if (!m_textureName) glGenTextures(1, &m_textureName);
        glBindTexture(GL_TEXTURE_RECTANGLE, m_textureName);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, &m_imageData[0]);
        glBindTexture(GL_TEXTURE_RECTANGLE, 0);
        ResetModified();

        // allocate framebuffer object
        if (!m_framebufferObject) glGenFramebuffers(1, &m_framebufferObject);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferObject);
        glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_textureName, 0);

        // check framebuffer status
        GLenum status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) std::cout << "ImageBuffer ERROR: Framebuffer object not complete!" << std::endl;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void ImageBuffer::Destroy() {
        if (m_framebufferObject) {
            glDeleteFramebuffers(1, &m_framebufferObject);
            m_framebufferObject = 0;
        }
        if (m_textureName) {
            glDeleteTextures(1, &m_textureName);
            m_textureName = 0;
        }
    }

    // --------------------------------------------------------------------------

    void ImageBuffer::SetPixel(int x, int y, glm::vec3 colour) {
        int index = y * m_width + x; // index into the buffer (2D grid indexed in 1D)
        m_imageData[index] = colour;

        // mark that something was changed
        m_modified = true;
        m_modifiedLower = std::min(m_modifiedLower, y);
        m_modifiedUpper = std::max(m_modifiedUpper, y+1);
    }

    // --------------------------------------------------------------------------

    // reference: https://stackoverflow.com/questions/5844858/how-to-take-screenshot-in-opengl
    // reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glReadPixels.xhtml
    // reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glReadBuffer.xhtml
    // reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glPixelStore.xhtml
    void ImageBuffer::readFromFrontBuffer() {
        unsigned int const NUMBER_OF_COLOUR_CHANNELS{3}; // RGB
        unsigned int const NUMBER_OF_PIXELS{(unsigned int)m_imageData.size()};
        GLubyte *pixels{new GLubyte[NUMBER_OF_PIXELS * NUMBER_OF_COLOUR_CHANNELS]};
        // read pixel data from lower-left corner of framebuffer (front) in row-order
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glReadBuffer(GL_FRONT);
        // reference: https://community.khronos.org/t/error-with-glreadpixels/20480/2
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        // reset
        glReadBuffer(GL_BACK);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 4);

        for (unsigned int y = 0; y < m_height; ++y) {
            for (unsigned int x = 0; x < m_width; ++x) {
                unsigned int const pixelIndex{y * m_width + x};
                SetPixel((int)x, (int)y, glm::vec3{pixels[NUMBER_OF_COLOUR_CHANNELS * pixelIndex] / 255.0f, pixels[NUMBER_OF_COLOUR_CHANNELS * pixelIndex + 1] / 255.0f, pixels[NUMBER_OF_COLOUR_CHANNELS * pixelIndex + 2] / 255.0f});
            }
        }

        delete[] pixels;
    }

    // --------------------------------------------------------------------------

    void ImageBuffer::Render() {
        if (!m_framebufferObject) return;

        // check for modifications to the image data and update texture as needed
        if (m_modified) {
            int sizeY = m_modifiedUpper - m_modifiedLower;
            int index = m_modifiedLower * m_width;

            // bind texture and copy only the rows that have been changed
            glBindTexture(GL_TEXTURE_RECTANGLE, m_textureName);
            glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, m_modifiedLower, m_width,
                            sizeY, GL_RGB, GL_FLOAT, &m_imageData[index]);
            glBindTexture(GL_TEXTURE_RECTANGLE, 0);

            // mark that we've updated the texture
            ResetModified();
        }

        // bind the framebuffer object with our texture in it and copy to screen
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferObject);
        glBlitFramebuffer(0, 0, m_width, m_height,
                        0, 0, m_width, m_height,
                        GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    // --------------------------------------------------------------------------

    bool ImageBuffer::SaveToFile(const std::string &imageFileName) {
        if (m_width == 0 || m_height == 0) {
            std::cout << "ImageBuffer ERROR: Trying to save uninitialized image!" << std::endl;
            return false;
        }
        std::cout << "ImageBuffer saving image to " << imageFileName << "..." << std::endl;

    #ifdef USE_STB_IMAGE
        const unsigned numComponents = 3; //RGB
        unsigned char* pixels = new unsigned char[m_width*m_height*numComponents];

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                glm::vec3& color = m_imageData[y * m_width + x];
                int i = (m_height - 1 - y) * m_width + x;
                i *= numComponents;

                pixels[i]     = (unsigned char) (255 * glm::clamp(color.r, 0.f, 1.f));	// red
                pixels[i + 1] = (unsigned char) (255 * glm::clamp(color.g, 0.f, 1.f));	// green
                pixels[i + 2] = (unsigned char) (255 * glm::clamp(color.b, 0.f, 1.f));	// blue
            }
        }

        // Save the image to disk
        int stride = 0;
        if (!stbi_write_png(imageFileName.data(), m_width, m_height, numComponents, pixels, stride)) {
            // Fail! delete and exit
            std::cout << "STB failed to write image " << imageFileName << std::endl;
            delete[] pixels;
            return false;
        }

        //success, delete and exit
        delete[] pixels;
        return true;
    #endif

    #ifdef USE_IMAGEMAGICK
        using namespace Magick;

        // allocate an image object the same size as our buffer
        Image myImage(Geometry(m_width, m_height), "black");

        // copy the image data from our memory buffer into the Magick++ one.
        int index = 0;
        for (int i = m_height-1; i >= 0; --i) {
            for (int j = 0; j < m_width; ++j) {
                glm::vec3 v = m_imageData[index++];
                glm::vec3 c = clamp(v, 0.f, 1.f) * float(MaxRGB);
                Color colour(c.r, c.g, c.b);
                myImage.pixelColor(j, i, colour);
            }
        }

        // try to write the image to the specified file
        try {
            myImage.write(imageFileName);
        } catch (Magick::Error &error) {
            std::cout << "Magick++ failed to write image " << imageFileName << std::endl;
            std::cout << "ERROR: " << error.what() << std::endl;
            return false;
        }
        return true;
    #endif

    #ifdef USE_FREEIMAGE
        FreeImage_Initialise();
        FIBITMAP* bitmap = FreeImage_Allocate(m_width, m_height, 24);
        if (bitmap == nullptr) {
            std::cout << "FreeImage failed to allocate image " << imageFileName << std::endl;
            FreeImage_DeInitialise();
            return false;
        }
        RGBQUAD colour;
        int index = 0;
        for (int i = 0; i < m_height; ++i) {
            for (int j = 0; j < m_width; ++j) {
                glm::vec3 v = m_imageData[index++];
                glm::vec3 c = clamp(v, 0.f, 1.f) * 255.0f;
                colour.rgbRed = (BYTE)c.r;
                colour.rgbGreen = (BYTE)c.g;
                colour.rgbBlue = (BYTE)c.b;
                FreeImage_SetPixelColor(bitmap, j, i, &colour);
            }
        }
        FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(imageFileName.c_str());
        if (fif == FIF_UNKNOWN) fif = FIF_PNG;
        if (!FreeImage_Save(fif, bitmap, imageFileName.c_str())) {
            std::cout << "FreeImage failed to write image " << imageFileName << std::endl;
            FreeImage_DeInitialise();
            return false;
        }
        FreeImage_DeInitialise();

        return true;
    #endif

        return false;
    }
}

// --------------------------------------------------------------------------
