#version 410 core

uniform vec4 fogColourFarAtCurrentTime;
uniform float fogDepthRadiusFar;
uniform float fogDepthRadiusNear;
uniform bool hasNormals;
uniform bool isTextured;
// in view-space
uniform vec3 lightVec;
uniform sampler2D textureData;
uniform float zFar;

in vec3 COLOUR;
in vec3 normalVec;
in vec2 UV;
in vec3 viewSpacePosition;
in vec3 viewVec;

out vec4 colour;

void main() {
    vec3 L = normalize(lightVec);
    vec3 N = normalize(normalVec);
    vec3 V = normalize(viewVec);
    //NOTE: using the view-space position since we want the distance from the camera eye (which is the origin of view-space)
    float worldSpaceDepth = clamp(length(viewSpacePosition) / zFar, 0.0f, 1.0f);

    vec4 baseColour = isTextured ? texture(textureData, UV) : vec4(COLOUR, 1.0f);
    // if we have normals, apply Lambertian diffuse
    // diffuse factor (in range [0.0, 1.0]
    const float K_D = 1.0f;
    // full-Lambert diffuse
    //vec3 diffuseColour = hasNormals ? K_D * clamp(dot(N, L), 0.0f, 1.0f) * baseColour.rgb : baseColour.rgb;
    // half-Lambert diffuse
    vec3 diffuseColour = hasNormals ? K_D * ((dot(N, L) + 1.0f) * 0.5f) * baseColour.rgb : baseColour.rgb;
    colour = vec4(diffuseColour, baseColour.a);

    //TODO: in future this will be moved out into a post-process shader program
    // apply fog...
    vec4 fogColour = fogColourFarAtCurrentTime;
    fogColour.a = worldSpaceDepth >= fogDepthRadiusFar ? fogColour.a : worldSpaceDepth <= fogDepthRadiusNear ? 0.0f : fogColour.a * ((worldSpaceDepth - fogDepthRadiusNear) / (fogDepthRadiusFar - fogDepthRadiusNear));
    //TODO: does this alpha make sense???
    colour = vec4(mix(colour.rgb, fogColour.rgb, fogColour.a), max(colour.a, fogColour.a));
}
