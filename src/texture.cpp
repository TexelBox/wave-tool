#include <cmath>
#include <iostream>
#include "texture.h"

namespace wave_tool {
    GLuint Texture::create2DTexture(unsigned char *data, unsigned int width, unsigned int height) {
        if (nullptr == data) return 0; // error code

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        // set options on currently bound texture object...
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // generate texture from data...
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // assuming texture is 32bits/pixel
        glGenerateMipmap(GL_TEXTURE_2D);

        return textureID;
    }

    void Texture::bind2DTexture(GLuint _program, GLuint _textureID, std::string const& varName) {
        glActiveTexture(GL_TEXTURE0 + _textureID);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glUniform1i(glGetUniformLocation(_program, varName.c_str()), _textureID);
    }

    void Texture::unbind2DTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
