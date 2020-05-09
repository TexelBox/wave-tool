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

// reference: https://stackoverflow.com/questions/2588875/whats-the-best-way-to-draw-a-fullscreen-quad-in-opengl-3-2
//NOTE: this shader expects an empty VAO to be bound before calling glDrawArrays(GL_TRIANGLE_STRIP, 0, 4)
//NOTE: the quad will be drawn with two triangles in a tri-strip format.
//      OpenGL automatically sets the winding of all triangles in a tri-strip to the winding of the first triangle.
//      reference: https://www.khronos.org/opengl/wiki/Primitive#Triangle_primitives
//      thus, the first three positions are in counter-clockwise order (default winding)
//NOTE: glViewport() can be used to map this quad to any sub-quad of screen, but most common case is a fullscreen quad
//NOTE: the X and Y coords are set to stretch corners to viewport bounds.
//      the Z coord doesn't matter if you have depth testing disabled (or just writing disabled), but I set it to 0.0 ("middle plane" of frustum) so that it's obvious when you forget to disable depth testing/writing
//      the W coord must be 1.0 in order for the true ndc-space position after perspective divide stage (gl_Position.xyz / gl_Position.w) to match the clip-space position (gl_Position.xyz)

const vec2 CORNER_UVS[4] = vec2[4](vec2(0.0f, 0.0f),  // bottom-left corner
                                   vec2(1.0f, 0.0f),  // bottom-right corner
                                   vec2(0.0f, 1.0f),  // top-left corner
                                   vec2(1.0f, 1.0f)); // top-right corner

const vec4 NDC_SPACE_CORNER_POSITIONS[4] = vec4[4](vec4(-1.0f, -1.0f, 0.0f, 1.0f), // bottom-left corner
                                                   vec4(1.0f, -1.0f, 0.0f, 1.0f),  // bottom-right corner
                                                   vec4(-1.0f, 1.0f, 0.0f, 1.0f),  // top-left corner
                                                   vec4(1.0f, 1.0f, 0.0f, 1.0f));  // top-right corner

out vec2 uv;

void main() {
    gl_Position = NDC_SPACE_CORNER_POSITIONS[gl_VertexID];
    uv = CORNER_UVS[gl_VertexID];
}
