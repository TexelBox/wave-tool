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

#include "mesh-object.h"

#include <glm/gtx/transform.hpp>

namespace wave_tool {
    MeshObject::MeshObject() :
        vao(0), vertexBuffer(0),
        normalBuffer(0), uvBuffer(0), colourBuffer(0),
        indexBuffer(0), textureID(0), shaderProgramID(0), hasTexture(false) {

        updateModel(); // init model matrix
    }

    MeshObject::~MeshObject() {
        // Remove data from GPU
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &uvBuffer);
        glDeleteBuffers(1, &normalBuffer);
        glDeleteBuffers(1, &colourBuffer);
        glDeleteBuffers(1, &indexBuffer);
        glDeleteVertexArrays(1, &vao);

        // delete the texture object since it never gets reused...
        glDeleteTextures(1, &textureID);
    }

    void MeshObject::updateModel() {
        // M = T*R*S
        // compute T, R, S first...
        glm::mat4 tMat = glm::translate(m_position);
        glm::mat4 rxMat = glm::rotate(glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ryMat = glm::rotate(glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rzMat = glm::rotate(glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rMat = rxMat * ryMat * rzMat; // Z then Y then X
        glm::mat4 sMat = glm::scale(m_scale);

        m_model = tMat * rMat * sMat; // S then R then T
    }

    //NOTE: this assumes counter-clockwise winding of triangular faces
    //NOTE: this method does not overwrite the normal buffer, it just overwrites the normal vector data
    void MeshObject::generateNormals() {
        if (PrimitiveMode::TRIANGLES != m_primitiveMode) return;

        // clear any old normal data...
        normals.clear();
        faceNormals.clear();

        // init (per-vertex normals) / (per-face normals)...
        normals.resize(drawVerts.size(), glm::vec3(0.0f, 0.0f, 0.0f));
        faceNormals.resize(drawFaces.size() / 3, glm::vec3(0.0f, 0.0f, 0.0f));

        // foreach triangle face in mesh...
        for (unsigned int f = 0; f < drawFaces.size(); f += 3) {
            // save the 3 vert indices making up face f...
            unsigned int const p1_index = drawFaces.at(f);
            unsigned int const p2_index = drawFaces.at(f + 1);
            unsigned int const p3_index = drawFaces.at(f + 2);

            // get the 3 vert positions...
            glm::vec3 const p1 = drawVerts.at(p1_index);
            glm::vec3 const p2 = drawVerts.at(p2_index);
            glm::vec3 const p3 = drawVerts.at(p3_index);

            // compute the outward face-normal (assuming CCW winding)...
            glm::vec3 const sideA = p2 - p1;
            glm::vec3 const sideB = p3 - p2;
            glm::vec3 const normal = glm::normalize(glm::cross(sideA, sideB));
            // SAFETY CHECK (e.g. if sideA or sideB were 0 vector OR if we tried to normalize the 0 vector)...
            // on error, this face has no contribution (could flag this face normal symbolically as the 0 vector)
            if (glm::any(glm::isnan(normal))) continue;

            faceNormals.at(f / 3) = normal;

            //TODO: in the future, it would be better to weight this contribution by the face angle, but for now just doing the trivial average technique
            normals.at(p1_index) += normal;
            normals.at(p2_index) += normal;
            normals.at(p3_index) += normal;
        }

        // normalize the accumulated normals...
        for (glm::vec3 &n : normals) {
            n = glm::normalize(n);
            // SAFETY CHECK (e.g. if n was somehow -nan or 0 vector...)
            // set this normal symbolically as 0 vector
            if (glm::any(glm::isnan(n))) n = glm::vec3(0.0f, 0.0f, 0.0f);
        }

    }
}
