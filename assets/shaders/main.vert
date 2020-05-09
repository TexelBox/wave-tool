#version 410 core

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

// default "plane" is a singularity which will cause this manual clipping test to always succeed for all vertices
//NOTE: this symbolic value should always be passed when you want this manual clipping disabled (cause some drivers might ignore glEnable/glDisable of GL_CLIP_DISTANCEi)
//NOTE: if you ever output a clip distance that isn't enabled, the clipping stage will just ignore the manual test
// reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_ClipDistance.xhtml
// reference: https://prideout.net/clip-planes
uniform vec4 clipPlane0 = vec4(0.0f, 0.0f, 0.0f, 1.0f); // <A, B, C, D> where Ax + By + Cz = D
uniform bool forceFlipNormals;
uniform mat4 modelMat;
uniform mat4 modelViewMat;
uniform mat4 mvpMat;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 colour;

out vec3 COLOUR;
out vec3 normalVec;
out vec2 UV;
out vec3 viewSpacePosition;
out vec3 viewVec;

out float gl_ClipDistance[1];

void main() {
    vec4 positionHomogenous = vec4(position, 1.0f);
    vec4 normalHomogenous = forceFlipNormals ? vec4(-normal, 0.0f) : vec4(normal, 0.0f);

    // output (pass-throughs)...
    COLOUR = colour;
    UV = uv;

    // output view-space position...
    viewSpacePosition = (modelViewMat * positionHomogenous).xyz;
    // output view vector...
    viewVec = normalize(viewSpacePosition);

    // output clip-space position...
    gl_Position = mvpMat * positionHomogenous;

    // apply manual clip plane...
    gl_ClipDistance[0] = dot(modelMat * positionHomogenous, clipPlane0);

    // output normal vector
    normalVec = normalize((modelViewMat * normalHomogenous).xyz);
}
