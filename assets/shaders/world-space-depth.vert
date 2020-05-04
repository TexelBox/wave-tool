#version 410 core

uniform mat4 modelViewMat;
uniform mat4 mvpMat;
uniform float zFar;

layout (location = 0) in vec3 position;

out float worldSpaceDepth;

void main() {
    vec4 positionHomogeneous = vec4(position, 1.0f);

    // convert the position into view-space so that we don't have to pass in the camera eye position (it is the origin of view-space)
    vec3 viewSpacePosition = (modelViewMat * positionHomogeneous).xyz;
    // output world-space depth...
    worldSpaceDepth = clamp(length(viewSpacePosition) / zFar, 0.0f, 1.0f);

    // output clip-space position...
    gl_Position = mvpMat * positionHomogeneous;
}
