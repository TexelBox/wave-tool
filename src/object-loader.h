#ifndef WAVE_TOOL_OBJECT_LOADER_H_
#define WAVE_TOOL_OBJECT_LOADER_H_

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

#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <cstring>
#include <cstdio>

#include "mesh-object.h"

namespace wave_tool {
    class ObjectLoader {
        public:
            // newer better loader that should be used
            //NOTE: will return indices starting from 0 (not 1 like obj format)
            //NOTE: assumes that all faces are triangles, otherwise returns false
            static bool loadTriMeshOBJ(std::string const& filePath, std::vector<glm::vec3> &out_verts, std::vector<glm::vec2> &out_uvs, std::vector<glm::vec3> &out_normals, std::vector<std::vector<glm::vec3>> &out_faces);

            static std::shared_ptr<MeshObject> createTriMeshObject(std::string const& filePath, bool const ignoreUVS = false, bool const ignoreNormals = false);
        private:

    };
}

#endif // WAVE_TOOL_OBJECT_LOADER_H_
