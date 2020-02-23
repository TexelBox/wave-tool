#version 410 core

uniform mat4 VPNoTranslation;

layout (location = 0) in vec3 vertex;

out vec3 STR;

void main() {
    STR = vec3(-vertex.x, vertex.yz); // flip the x component to mirror texture (since we are inside box)
    vec4 pos = VPNoTranslation * vec4(vertex, 1.0f);
    gl_Position = pos;
}
