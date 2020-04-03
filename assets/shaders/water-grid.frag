#version 410 core

uniform samplerCube skybox;

in vec4 heightmap_colour;
in vec3 normal;
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
    //TODO: do I need to handle a negative value here???
    float fresnel_cos_theta = dot(normal, viewVec);
    float fresnel_f_theta = fresnel_f_0 + (1.0f - fresnel_f_0) * pow(1.0f - fresnel_cos_theta, 5);

    //NOTE: this is currently the same colour thats in the paper's demo application
    //TODO: pass this in as a uniform that can be changed in the UI or by hard-coded scenes
    vec4 water_tint_colour = vec4(0.17f, 0.27f, 0.26f, 1.0f);

    // output final fragment colour...
    // reference: https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/projgrid-hq.pdf
    //TODO: expand this later to combine other visual effects
    colour = (1.0f - fresnel_f_theta) * water_tint_colour + fresnel_f_theta * skybox_reflection_colour;

    //TODO: have a UI toggle for this debug colour...
    //colour = heightmap_colour;
}
