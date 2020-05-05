#version 410 core

uniform float zFar;

in vec3 viewSpacePosition;

out vec4 colour;

void main() {
    //NOTE: using the view-space position since we want the distance from the camera eye (which is the origin of view-space)
    float worldSpaceDepth = clamp(length(viewSpacePosition) / zFar, 0.0f, 1.0f);

    // output fragment colour...
    //NOTE: the alpha here should always be 1.0 and the clear colour alpha can be 0.0
    colour = vec4(worldSpaceDepth, worldSpaceDepth, worldSpaceDepth, 1.0f);
}
