#version 410 core

uniform mat4 mvpMat;

layout (location = 0) in vec3 position;

void main() {
    vec4 positionHomogeneous = vec4(position, 1.0f);
    // output clip-space position...
    gl_Position = mvpMat * positionHomogeneous;
}
