#version 410 core

uniform sampler2D image;
uniform bool hasTexture;
uniform bool hasNormals;

in vec3 N;
in vec3 L;
in vec3 V;
in vec2 UV;
in vec3 COLOUR;

in vec3 sunPosition;
in float viewDepth;

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

    // apply hard-coded fog...
    //TODO: allow sun colouring of the fog?
    //TODO: add UI setting for fog density
    // reference: http://in2gpu.com/2014/07/22/create-fog-shader/
    //TODO: pass this by uniform?
    vec3 fogColour = mix(vec3(0.3f, 0.3f, 0.3f), vec3(1.0f, 1.0f, 1.0f), clamp(sunPosition.y, 0.0f, 1.0f));
    // the attenuation factor (b)
    const float FOG_DENSITY = 0.0002f;
    // exponential fog
    float f_fog = exp(-(viewDepth * FOG_DENSITY));
    // exponential-squared fog
    //float f_fog = exp(-pow(viewDepth * FOG_DENSITY, 2));

    colour = vec4(mix(fogColour, colour.xyz, f_fog), 1.0f);
}
