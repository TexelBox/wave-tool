#version 410 core

uniform samplerCube skyboxStars;

in vec3 STR;

out vec4 colour;

void main() {
    colour = vec4(texture(skyboxStars, STR).rgb, 1.0f);

    // debug colours...
    // texture coordinate visualization
    //colour = vec4(STR, 1.0f);
}
