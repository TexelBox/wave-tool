#ifndef WAVE_TOOL_OBJECT_LOADER_H_
#define WAVE_TOOL_OBJECT_LOADER_H_

#include <iostream>
#include <map>
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
