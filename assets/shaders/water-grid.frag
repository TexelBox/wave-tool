#version 410 core

uniform vec4 fogColourFarAtCurrentTime;
uniform float fogDepthRadiusFar;
uniform float fogDepthRadiusNear;
uniform samplerCube skybox;
// the inverse light direction
uniform vec3 sunPosition;
// higher shininess means smaller specular highlight (sun)
uniform float sunShininess;
// this scalar affects how much of the sun light is added on top of the diffuse sky colour
uniform float sunStrength;

in vec4 heightmap_colour;
in vec3 normal;
in vec3 viewVec;
in float viewVecDepth;

out vec4 colour;

void main() {
    // global reflections (from skybox)...
    // reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/reflect.xhtml
    // both input vectors should be normalized to ensure output vector is normalized
    //NOTE: the incident vector must point towards the surface (thus, we negate the view vector that is defined as pointing away)
    vec3 R = reflect(-viewVec, normal);
    vec4 skybox_reflection_colour = vec4(texture(skybox, R).rgb, 1.0f);

    //NOTE: should be same value as in skysphere shader
    const vec3 SUN_BASE_COLOUR = vec3(1.0f, 1.0f, 1.0f);
    vec4 sun_reflection_colour = vec4(sunStrength * SUN_BASE_COLOUR * pow(clamp(dot(R, sunPosition), 0.0f, 1.0f), sunShininess), 1.0f);

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

    //TODO: change this to something more accurate later (like account for ocean depth?), this is just a test for now
    //TODO: should pass this colour by uniform if it is not gonna vary with different fragments

    vec4 water_tint_colour = vec4(mix(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.341f, 0.482f), clamp(sunPosition.y, 0.0f, 1.0f)), 1.0f);

    // output final fragment colour...
    // reference: https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/projgrid-hq.pdf
    //TODO: expand this later to combine other visual effects
    //TODO: currently there are visual artifacts when the camera is within the displaceable volume, but its minor and rarely noticeable
    colour = (1.0f - fresnel_f_theta) * water_tint_colour + fresnel_f_theta * (skybox_reflection_colour + sun_reflection_colour);

    //TODO: in future this will be moved out into a post-process shader program
    // apply fog...
    vec4 fogColour = fogColourFarAtCurrentTime;
    fogColour.a = viewVecDepth >= fogDepthRadiusFar ? fogColour.a : viewVecDepth <= fogDepthRadiusNear ? 0.0f : fogColour.a * ((viewVecDepth - fogDepthRadiusNear) / (fogDepthRadiusFar - fogDepthRadiusNear));
    colour = vec4(mix(colour.rgb, fogColour.rgb, fogColour.a), max(colour.a, fogColour.a));

    //TODO: have a UI toggle for this debug colour...
    //colour = heightmap_colour;
}
