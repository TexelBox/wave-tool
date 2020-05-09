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

// this is the sky-gradient texture that will be interpolated based on time of day
uniform sampler1D skysphere;
// this scales the sun horizon colour to be darker
uniform float sunHorizonDarkness;
// the inverse light direction
uniform vec3 sunPosition;
// higher shininess means smaller specular highlight (sun)
uniform float sunShininess;
// this scalar affects how much of the sun light is added on top of the diffuse sky colour
uniform float sunStrength;

in vec3 normalVec;

out vec4 colour;

void main() {
    //NOTE: both vectors currently in world-space
    vec3 L = normalize(sunPosition);
    vec3 N = normalize(normalVec);

    float a_1 = 0.25f * (L.y + 1.0f);
    float a_2 = 1.0f - a_1;
    vec3 sunPeakColour = texture(skysphere, a_1).rgb;
    vec3 sunHorizonColour = mix(texture(skysphere, a_2).rgb, vec3(0.0f, 0.0f, 0.0f), sunHorizonDarkness);

    float a_3 = dot(N, L);
    vec3 diffuseColour = a_3 >= 0.0f ? mix(sunHorizonColour, sunPeakColour, a_3) : mix(sunHorizonColour, vec3(0.0f, 0.0f, 0.0f), abs(a_3) * (1.0f - abs(L.y)));

    // the sun is drawn as a specular hightlight that will be same (or similar) as the reflection on the water
    const vec3 SUN_BASE_COLOUR = vec3(1.0f, 1.0f, 1.0f);
    vec3 specularColour = a_3 > 0.0f ? sunStrength * SUN_BASE_COLOUR * pow(a_3, sunShininess) : vec3(0.0f, 0.0f, 0.0f);

    // when L.y >= this, then this fragment will be fully opaque. Thus this should be in range [-1.0, 1.0]
    //NOTE: this is used to make sure no stars are seen during the day
    const float FULLY_OPAQUE_THRESHOLD = 0.1f;
    const float FULLY_OPAQUE_SHIFT = clamp(1.0f - FULLY_OPAQUE_THRESHOLD, 0.0f, 2.0f);
    //NOTE: (L.y + 1.0f) / (FULLY_OPAQUE_THRESHOLD + 1.0f) is a mapping to the range [0.0, 1.0) where it interpolates from L.y == -1.0 (inclusive) upto L.y == FULLY_OPAQUE_THRESHOLD (exclusive)
    float skysphereAlpha_1 = L.y < FULLY_OPAQUE_THRESHOLD ? (L.y + 1.0f) / (FULLY_OPAQUE_THRESHOLD + 1.0f) : 1.0f;
    //NOTE: this second factor is used to scale stars to be duller when close to sun position
    float skysphereAlpha_2 = (a_3 + 1.0f) / 2.0f;
    float skysphereAlpha = max(skysphereAlpha_1, skysphereAlpha_2);
    //TODO: could add a clamped lower bound in order to simulate nightime overcast/fog, but everything looks good regardless
    colour = vec4(diffuseColour + specularColour, skysphereAlpha);

    // debug colours...
    // texture coordinate visualization
    //colour = vec4(worldPosition, 1.0f);
}
