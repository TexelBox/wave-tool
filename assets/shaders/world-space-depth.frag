#version 410 core

in float worldSpaceDepth;

out vec4 colour;

void main() {
    //NOTE: this should already be clamped, but this is for safety
    float worldSpaceDepthClamped = clamp(worldSpaceDepth, 0.0f, 1.0f);
    // output fragment colour...
    //NOTE: the alpha here should always be 1.0 and the clear colour alpha can be 0.0
    colour = vec4(worldSpaceDepthClamped, worldSpaceDepthClamped, worldSpaceDepthClamped, 1.0f);
}
