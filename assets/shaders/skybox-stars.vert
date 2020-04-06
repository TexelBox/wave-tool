#version 410 core

uniform mat4 VPNoTranslation;

layout (location = 0) in vec3 vertex;

out vec3 STR;

void main() {
    // normalize the vertex position to ensure we have a unit-diagonal cube
    vec3 vertexNormalized = normalize(vertex);

    // output texture coords
    STR = vertexNormalized;

    // output position in clip-space
    gl_Position = VPNoTranslation * vec4(vertexNormalized, 1.0f);
}
