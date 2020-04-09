#version 410 core

uniform samplerCube skybox;
uniform sampler1D skysphere;
uniform vec3 sunPosition;

in vec4 heightmap_colour;
in vec3 normal;
in float viewDepth;
in vec3 viewVec;

out vec4 colour;

void main() {
    // global reflections (from skybox)...
    // reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/reflect.xhtml
    // both input vectors should be normalized to ensure output vector is normalized
    //NOTE: the incident vector must point towards the surface (thus, we negate the view vector that is defined as pointing away)
    vec3 R = reflect(-viewVec, normal);
    vec4 skybox_reflection_colour = vec4(texture(skybox, R).rgb, 1.0f);

    // fresnel reflectance (schlick approximation...)
    // reference: https://en.wikipedia.org/wiki/Schlick%27s_approximation
    // this is the reflection coefficient for light incoming parallel to the normal (thus, minimal reflectance)
    // since we are always working with two mediums - air (n_1 = 1.0003) and water (n_2 = 1.33), we can precompute this constant from the following formula...
    // ---> f_0 = ((n_1 - n_2) / (n_1 + n_2))^2 = 0.02 (almost perfectly transmissive at this angle)  
    float fresnel_f_0 = 0.02f;
    // both input vectors should be normalized
    //NOTE: since the vertex shader flips the normal when the camera is underwater, this dot product will be in range [0.0, 1.0]
    float fresnel_cos_theta = dot(normal, viewVec);
    float fresnel_f_theta = fresnel_f_0 + (1.0f - fresnel_f_0) * pow(1.0f - fresnel_cos_theta, 5);

    //TEMPORARY...
    //TODO: change this to something more accurate later, this is just a test for now
    //TODO: should pass this colour by uniform if it is not gonna vary with different fragments
    float a_1 = 0.25f * (sunPosition.y + 1.0f);
    float a_2 = 1.0f - a_1;
    //NOTE: this doesn't factor in the sun horizon darkness like the skysphere does! 
    vec3 sunHorizonColour = mix(texture(skysphere, a_2).rgb, vec3(0.0f, 0.0f, 0.0f), 0.25f);

    //NOTE: this is currently the same colour thats in the paper's demo application
    //vec4 water_tint_colour = vec4(0.17f, 0.27f, 0.26f, 1.0f);
    vec4 water_tint_colour = vec4(sunHorizonColour, 1.0f);

    // output final fragment colour...
    // reference: https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/projgrid-hq.pdf
    //TODO: expand this later to combine other visual effects
    //TODO: currently there are visual artifacts when the camera is within the displaceable volume, but its minor and rarely noticeable
    colour = (1.0f - fresnel_f_theta) * water_tint_colour + fresnel_f_theta * skybox_reflection_colour;

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

    //TODO: have a UI toggle for this debug colour...
    //colour = heightmap_colour;
}
