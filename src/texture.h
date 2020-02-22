#ifndef WAVE_TOOL_TEXTURE_H_
#define WAVE_TOOL_TEXTURE_H_

#include <glad/glad.h>
#include <vector>
#include <string>

namespace wave_tool {
    class Texture {
        public:
            static GLuint create2DTexture(std::vector<unsigned char> &image, unsigned int width, unsigned int height);

            static void bind2DTexture(GLuint _program, GLuint _textureID, std::string varName);

            static void unbind2DTexture();
    };
}

#endif // WAVE_TOOL_TEXTURE_H_
