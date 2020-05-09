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

#include <cmath>
#include <iostream>
#include "texture.h"

namespace wave_tool {
    GLuint Texture::create1DTexture(unsigned char *data, unsigned int length) {
        if (nullptr == data) return 0; // error code

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_1D, textureID);
        // set options on currently bound texture object...
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // generate texture from data...
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, length, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // assuming texture is 32bits/pixel
        glGenerateMipmap(GL_TEXTURE_1D);

        return textureID;
    }

    GLuint Texture::create2DTexture(unsigned char *data, unsigned int width, unsigned int height) {
        if (nullptr == data) return 0; // error code

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        // set options on currently bound texture object...
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // generate texture from data...
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // assuming texture is 32bits/pixel
        glGenerateMipmap(GL_TEXTURE_2D);

        return textureID;
    }

    void Texture::bind1DTexture(GLuint _program, GLuint _textureID, std::string const& varName) {
        glActiveTexture(GL_TEXTURE0 + _textureID);
        glBindTexture(GL_TEXTURE_1D, _textureID);
        glUniform1i(glGetUniformLocation(_program, varName.c_str()), _textureID);
    }

    void Texture::bind2DTexture(GLuint _program, GLuint _textureID, std::string const& varName) {
        glActiveTexture(GL_TEXTURE0 + _textureID);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glUniform1i(glGetUniformLocation(_program, varName.c_str()), _textureID);
    }

    void Texture::unbind1DTexture() {
        glBindTexture(GL_TEXTURE_1D, 0);
    }

    void Texture::unbind2DTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
