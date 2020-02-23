#version 410 core

uniform samplerCube skybox;

in vec3 STR;

out vec4 colour;

void main() {
    colour = texture(skybox, STR);
}
