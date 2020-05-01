#version 410 core

// simple shader that either samples from a 2D texture or uses a solid colour for all fragments

uniform bool isTextured;
uniform vec4 solidColour;
uniform sampler2D textureData;

in vec2 uv;

out vec4 colour;

void main() {
    if (isTextured) colour = texture(textureData, uv);
    else colour = solidColour;
}
