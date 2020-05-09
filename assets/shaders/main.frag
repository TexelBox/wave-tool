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

uniform vec4 fogColourFarAtCurrentTime;
uniform float fogDepthRadiusFar;
uniform float fogDepthRadiusNear;
uniform bool hasNormals;
uniform bool isTextured;
// in view-space
uniform vec3 lightVec;
uniform sampler2D textureData;
uniform float zFar;

in vec3 COLOUR;
in vec3 normalVec;
in vec2 UV;
in vec3 viewSpacePosition;
in vec3 viewVec;

out vec4 colour;

void main() {
    vec3 L = normalize(lightVec);
    vec3 N = normalize(normalVec);
    vec3 V = normalize(viewVec);
    //NOTE: using the view-space position since we want the distance from the camera eye (which is the origin of view-space)
    float worldSpaceDepth = clamp(length(viewSpacePosition) / zFar, 0.0f, 1.0f);

    vec4 baseColour = isTextured ? texture(textureData, UV) : vec4(COLOUR, 1.0f);
    // if we have normals, apply Lambertian diffuse
    // diffuse factor (in range [0.0, 1.0]
    const float K_D = 1.0f;
    // full-Lambert diffuse
    //vec3 diffuseColour = hasNormals ? K_D * clamp(dot(N, L), 0.0f, 1.0f) * baseColour.rgb : baseColour.rgb;
    // half-Lambert diffuse
    vec3 diffuseColour = hasNormals ? K_D * ((dot(N, L) + 1.0f) * 0.5f) * baseColour.rgb : baseColour.rgb;
    colour = vec4(diffuseColour, baseColour.a);

    //TODO: in future this will be moved out into a post-process shader program
    // apply fog...
    vec4 fogColour = fogColourFarAtCurrentTime;
    fogColour.a = worldSpaceDepth >= fogDepthRadiusFar ? fogColour.a : worldSpaceDepth <= fogDepthRadiusNear ? 0.0f : fogColour.a * ((worldSpaceDepth - fogDepthRadiusNear) / (fogDepthRadiusFar - fogDepthRadiusNear));
    //TODO: does this alpha make sense???
    colour = vec4(mix(colour.rgb, fogColour.rgb, fogColour.a), max(colour.a, fogColour.a));
}
