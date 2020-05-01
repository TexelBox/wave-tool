#version 410 core

uniform vec4 fogColourFarAtCurrentTime;
uniform float fogDepthRadiusFar;
uniform float fogDepthRadiusNear;
uniform sampler2D image;
uniform bool hasTexture;
uniform bool hasNormals;

in vec3 N;
in vec3 L;
in vec3 V;
in vec2 UV;
in vec3 COLOUR;

in vec3 sunPosition;
in float viewVecDepth;

out vec4 colour;

//TODO: refactor this shader
void main(void) {

    /***** Image-based texturing *****/
    vec4 imgColour;

    if (hasTexture) {
        imgColour = texture(image, UV);
    } else {
        imgColour = vec4(COLOUR, 1.0f);
    }

    if (hasNormals) {
        float diffuse =  (dot(N, L) + 1) / 2;
        vec3 diffuseColour = diffuse * vec3(imgColour.x, imgColour.y, imgColour.z);

        colour = vec4(diffuseColour, imgColour.w);
    } else {
        colour = imgColour;
    }

    //TODO: in future this will be moved out into a post-process shader program
    // apply fog...
    vec4 fogColour = fogColourFarAtCurrentTime;
    fogColour.a = viewVecDepth >= fogDepthRadiusFar ? fogColour.a : viewVecDepth <= fogDepthRadiusNear ? 0.0f : fogColour.a * ((viewVecDepth - fogDepthRadiusNear) / (fogDepthRadiusFar - fogDepthRadiusNear));
    colour = vec4(mix(colour.rgb, fogColour.rgb, fogColour.a), max(colour.a, fogColour.a));
}
