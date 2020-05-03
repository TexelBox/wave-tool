#version 410 core

uniform vec4 fogColourFarAtCurrentTime;
uniform float fogDepthRadiusFar;
uniform float fogDepthRadiusNear;
uniform sampler2D localReflectionsTexture2D;
uniform sampler2D localRefractionsTexture2D;
uniform samplerCube skybox;
// the inverse light direction
uniform vec3 sunPosition;
// higher shininess means smaller specular highlight (sun)
uniform float sunShininess;
// this scalar affects how much of the sun light is added on top of the diffuse sky colour
uniform float sunStrength;
uniform vec2 viewportWidthHeight;

in vec4 heightmap_colour;
in vec3 normal;
in vec3 normalVecInViewSpaceOnlyYaw;
in vec3 viewVec;
in float viewVecDepth;
in vec2 xyPositionNDCSpaceHeight0;

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

    // LOCAL REFLECTIONS/REFRACTIONS...
    // compute the two starting uvs, that only differ by their world-space y component (height)...
    // convert viewport-space coords to uv-space coords
    vec2 uvViewportSpace = gl_FragCoord.xy / viewportWidthHeight;
    // convert ndc-space coords to uv-space coords
    vec2 uvViewportSpaceHeight0 = (xyPositionNDCSpaceHeight0 + vec2(1.0f, 1.0f)) / 2.0f;

    // compute the amount of distortion based on product of 1. empirical scalar and 2. view vector depth...
    // the view vector depth is factored in since the surface should look flatter farther away
    //NOTE: the distortion will automatically scale with the viewport size, since UV-space is always 0.0 to 1.0, but will span over different resolutions
    //NOTE: these scalars must be in range [0.0, 1.0]
    const float LOCAL_REFLECTIONS_DISTORTION_STRENGTH = 0.1f;
    const float LOCAL_REFRACTIONS_DISTORTION_STRENGTH = 0.1f;
    // this should already be clamped, but doing this for safety
    float viewVecDepthClamped = clamp(viewVecDepth, 0.0f, 1.0f);    
    float localReflectionsDistortionScalar = LOCAL_REFLECTIONS_DISTORTION_STRENGTH * (1.0f - viewVecDepthClamped);
    float localRefractionsDistortionScalar = LOCAL_REFRACTIONS_DISTORTION_STRENGTH * (1.0f - viewVecDepthClamped);

    // compute the distorted uv-space coords...
    vec2 uvLocalReflectionsYDistorted = mix(uvViewportSpace, uvViewportSpaceHeight0, localReflectionsDistortionScalar);
    vec2 uvLocalRefractionsYDistorted = mix(uvViewportSpace, uvViewportSpaceHeight0, localRefractionsDistortionScalar);
    vec2 uvLocalReflectionsXZDistortion = localReflectionsDistortionScalar * vec2(normalVecInViewSpaceOnlyYaw.x, -normalVecInViewSpaceOnlyYaw.z);
    vec2 uvLocalRefractionsXZDistortion = localRefractionsDistortionScalar * vec2(-normalVecInViewSpaceOnlyYaw.x, normalVecInViewSpaceOnlyYaw.z);
    vec2 uvLocalReflections = clamp(uvLocalReflectionsYDistorted + uvLocalReflectionsXZDistortion, vec2(0.0f, 0.0f), vec2(1.0f, 1.0f));
    vec2 uvLocalRefractions = clamp(uvLocalRefractionsYDistorted + uvLocalRefractionsXZDistortion, vec2(0.0f, 0.0f), vec2(1.0f, 1.0f));

    // finally, we can compute the local reflection and refraction colours (where an alpha of 0.0 symbolically represents no local reflection (or local refraction, respectively) for this fragment)
    vec4 localReflectionColour = texture(localReflectionsTexture2D, uvLocalReflections);
    vec4 localRefractionColour = texture(localRefractionsTexture2D, uvLocalRefractions);

    //TODO: change this to something more accurate later (like account for ocean depth?), this is just a test for now
    //TODO: should pass this colour by uniform if it is not gonna vary with different fragments
    vec4 deepestWaterColour = vec4(mix(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.341f, 0.482f), clamp(sunPosition.y, 0.0f, 1.0f)), 1.0f);

    // output final fragment colour...
    // reference: https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/projgrid-hq.pdf
    //TODO: expand this later to combine other visual effects
    //TODO: currently there are visual artifacts when the camera is within the displaceable volume, but its minor and rarely noticeable
    //TODO: mix the deepestWaterColour with the localRefractionColour based on a view depth delta (multipied by the alpha?)
    //NOTE: currently just clamping the localRefractionColour.a in order to see a close representation of what its gonna look like with the depth colouring
    colour = vec4(mix(mix(deepestWaterColour.rgb, localRefractionColour.rgb, clamp(localRefractionColour.a, 0.0f, 0.3f)), mix(skybox_reflection_colour.rgb + sun_reflection_colour.rgb, localReflectionColour.rgb, localReflectionColour.a), fresnel_f_theta), 1.0f);

    //TODO: in future this will be moved out into a post-process shader program
    // apply fog...
    vec4 fogColour = fogColourFarAtCurrentTime;
    fogColour.a = viewVecDepth >= fogDepthRadiusFar ? fogColour.a : viewVecDepth <= fogDepthRadiusNear ? 0.0f : fogColour.a * ((viewVecDepth - fogDepthRadiusNear) / (fogDepthRadiusFar - fogDepthRadiusNear));
    colour = vec4(mix(colour.rgb, fogColour.rgb, fogColour.a), max(colour.a, fogColour.a));

    //TODO: have a UI toggle for this debug colour...
    //colour = heightmap_colour;
}
