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

#include "shader-tools.h"

namespace wave_tool {
    GLuint ShaderTools::compileShaders(char const* vertexFilename, char const* fragmentFilename) {
        GLuint vertex_shader;
        GLuint fragment_shader;
        GLuint program;

        GLchar const*vertex_shader_source[] = {loadshader(vertexFilename)};
        GLchar const*fragment_shader_source[] = {loadshader(fragmentFilename)};

        // Create and compile vertex shader
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, vertex_shader_source, nullptr);
        glCompileShader(vertex_shader);

        //Create and compile a fragment shader
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, fragment_shader_source, nullptr);
        glCompileShader(fragment_shader);

        // Create program, attach shaders to it, and link it
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);

        glLinkProgram(program);

        GLint status;
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);

        if (GL_FALSE == status) {
            GLint infoLogLength;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            GLchar *strInfoLog = new GLchar[infoLogLength + 1];
            glGetShaderInfoLog(fragment_shader, infoLogLength, nullptr, strInfoLog);

            fprintf(stderr, "Compilation error in shader fragment_shader: %s\n", strInfoLog);
            delete[] strInfoLog;
        }

        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);

        if (GL_FALSE == status) {
            GLint infoLogLength;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            GLchar *strInfoLog = new GLchar[infoLogLength + 1];
            glGetShaderInfoLog(vertex_shader, infoLogLength, nullptr, strInfoLog);

            fprintf(stderr, "Compilation error in shader vertex_shader: %s\n", strInfoLog);
            delete[] strInfoLog;
        }

        // Delete the shaders as the program has them now
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        unloadshader((GLchar**)vertex_shader_source);
        unloadshader((GLchar**)fragment_shader_source);

        return program;
    }

    GLuint ShaderTools::compileShaders(char const* vertexFilename, char const* geometryFilename, char const* fragmentFilename) {
        GLuint vertex_shader;
        GLuint fragment_shader;
        GLuint geometry_shader;

        GLuint program;

        GLchar const*vertex_shader_source[] = {loadshader(vertexFilename)};
        GLchar const*fragment_shader_source[] = {loadshader(fragmentFilename)};
        GLchar const*geometry_shader_source[] = {loadshader(geometryFilename)};

        // Create and compile the vertex shader
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, vertex_shader_source, nullptr);
        glCompileShader(vertex_shader);

        //Create and compile the fragment shader
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, fragment_shader_source, nullptr);
        glCompileShader(fragment_shader);

        //Create and compile the geometry shader
        geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry_shader, 1, geometry_shader_source, nullptr);
        glCompileShader(geometry_shader);

        // Create program, attach shaders to it, and link it
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, geometry_shader);
        glAttachShader(program, fragment_shader);

        glLinkProgram(program);

        GLint status;
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);

        if (GL_FALSE == status) {
            GLint infoLogLength;
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            GLchar *strInfoLog = new GLchar[infoLogLength + 1];
            glGetShaderInfoLog(fragment_shader, infoLogLength, nullptr, strInfoLog);

            fprintf(stderr, "Compilation error in shader fragment_shader: %s\n", strInfoLog);
            delete[] strInfoLog;
        }

        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);

        if (GL_FALSE == status) {
            GLint infoLogLength;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            GLchar *strInfoLog = new GLchar[infoLogLength + 1];
            glGetShaderInfoLog(vertex_shader, infoLogLength, nullptr, strInfoLog);

            fprintf(stderr, "Compilation error in shader vertex_shader: %s\n", strInfoLog);
            delete[] strInfoLog;
        }

        glGetShaderiv(geometry_shader, GL_COMPILE_STATUS, &status);

        if (GL_FALSE == status) {
            GLint infoLogLength;
            glGetShaderiv(geometry_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            GLchar *strInfoLog = new GLchar[infoLogLength + 1];
            glGetShaderInfoLog(geometry_shader, infoLogLength, nullptr, strInfoLog);

            fprintf(stderr, "Compilation error in shader geometry_shader: %s\n", strInfoLog);
            delete[] strInfoLog;
        }

        // Delete the shaders as the program has them now
        glDeleteShader(vertex_shader);
        glDeleteShader(geometry_shader);
        glDeleteShader(fragment_shader);

        unloadshader((GLchar**)vertex_shader_source);
        unloadshader((GLchar**)geometry_shader_source);
        unloadshader((GLchar**)fragment_shader_source);

        return program;
    }

    unsigned long ShaderTools::getFileLength(std::ifstream &file) {
        if (!file.good()) return 0;

        file.seekg(0, std::ios::end);
        unsigned long len = file.tellg();
        file.seekg(std::ios::beg);

        return len;
    }

    GLchar* ShaderTools::loadshader(std::string filename) {
        std::ifstream file;
        file.open(filename.c_str(), std::ios::in); // opens as ASCII!
        if (!file) return nullptr;

        unsigned long len = getFileLength(file);

        if (0 == len) return nullptr; // Error: Empty File

        GLchar *ShaderSource = nullptr;
        ShaderSource = new char[len + 1];
        if (nullptr == ShaderSource) return nullptr; // can't reserve memoryf

        // len isn't always strlen cause some characters are stripped in ascii read...
        // it is important to 0-terminate the real length later, len is just max possible value...
        ShaderSource[len] = 0;

        unsigned int i = 0;
        while (file.good()) {
            ShaderSource[i] = file.get(); // get character from file.
            if (!file.eof()) ++i;
        }

        ShaderSource[i] = 0; // 0-terminate it at the correct position

        file.close();

        return ShaderSource; // No Error
    }

    void ShaderTools::unloadshader(GLchar **ShaderSource) {
        if (nullptr != *ShaderSource) {
            delete[] *ShaderSource;
        }
        *ShaderSource = nullptr;
    }
}
