#include <cmath>
#include <iostream>
#include "texture.h"

namespace wave_tool {
    GLuint Texture::create2DTexture(std::vector<unsigned char> &image, unsigned int width, unsigned int height) {
        GLuint textureID;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // only available in OpenGL 4.2+
        //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

        //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data()); // idk if this is fully correct, but it works!
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        return textureID;
    }

    void Texture::bind2DTexture(GLuint _program, GLuint _textureID, std::string varName) {
        glActiveTexture(GL_TEXTURE0 + _textureID);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glUniform1i(glGetUniformLocation(_program, varName.c_str()), _textureID);
    }

    void Texture::unbind2DTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
