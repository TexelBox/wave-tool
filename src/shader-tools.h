#ifndef WAVE_TOOL_SHADER_TOOLS_H_
#define WAVE_TOOL_SHADER_TOOLS_H_

#include <glad/glad.h>
#include <iostream>
#include <fstream>

namespace wave_tool {
    // Class modified from code provided by Allan Rocha for CPSC 591
    class ShaderTools {
        public:
            static GLuint compileShaders(char const* vertexFilename, char const* fragmentFilename);
            static GLuint compileShaders(char const* vertexFilename, char const* geometryFilename, char const* fragmentFilename);
        private:
            static unsigned long getFileLength(std::ifstream &file);
            static GLchar* loadshader(std::string filename);
            static void unloadshader(GLchar **ShaderSource);
    };
}

#endif // WAVE_TOOL_SHADER_TOOLS_H_
