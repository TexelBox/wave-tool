#version 410 core

uniform mat4 VPNoTranslation;

layout (location = 0) in vec3 vertex;

out vec3 STR;

void main() {
    STR = vertex;
    vec4 pos = VPNoTranslation * vec4(vertex, 1.0f);
    gl_Position = pos;
}
