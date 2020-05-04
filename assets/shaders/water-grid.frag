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
uniform sampler2D worldSpaceDepthTexture2D;

in vec4 heightmap_colour;
in vec3 normal;
in vec3 normalVecInViewSpaceOnlyYaw;
in vec3 viewVec;
in float viewVecDepth;
in vec2 xyPositionNDCSpaceHeight0;

out vec4 colour;

void main() {
    // compute the two starting uvs, that only differ by their world-space y component (height)...
    // convert viewport-space coords to uv-space coords
    vec2 uvViewportSpace = gl_FragCoord.xy / viewportWidthHeight;
    // convert ndc-space coords to uv-space coords
    vec2 uvViewportSpaceHeight0 = (xyPositionNDCSpaceHeight0 + vec2(1.0f, 1.0f)) / 2.0f;

    // get world-space depth of fragment currently in back buffer
    vec4 worldSpaceDepthFragBackColour = texture(worldSpaceDepthTexture2D, uvViewportSpace);
    float worldSpaceDepthFragBack = worldSpaceDepthFragBackColour.r;

    // compute the difference in world-space depth between this fragment and the back buffer fragment...
    // this should already be clamped, but doing this for safety
    float viewVecDepthClamped = clamp(viewVecDepth, 0.0f, 1.0f);
    // in range [-1.0, 1.0]
    float deltaWorldSpaceDepth = worldSpaceDepthFragBack - viewVecDepthClamped;
    // clamp the negative deltas (this fragment is behind back buffer fragment)
    float deltaWorldSpaceDepthClamped = clamp(deltaWorldSpaceDepth, 0.0f, 1.0f);

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
    // compute the amount of distortion based on product of 1. empirical scalar and 2. view vector depth...
    // the view vector depth is factored in since the surface should look flatter farther away
    //NOTE: the distortion will automatically scale with the viewport size, since UV-space is always 0.0 to 1.0, but will span over different resolutions
    //NOTE: these scalars must be in range [0.0, 1.0]
    const float LOCAL_REFLECTIONS_DISTORTION_STRENGTH = 0.1f;
    const float LOCAL_REFRACTIONS_DISTORTION_STRENGTH = 0.1f;
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

    //TODO: figure out a more realistic way in determining the tint colour (e.g. factor in the sky colour?), or have them in UI?
    const vec3 DEEP_TINT_COLOUR_AT_NOON = vec3(0.0f, 0.341f, 0.482f);
    const vec3 SHALLOW_TINT_COLOUR_AT_NOON = vec3(0.3f, 0.941f, 0.903f);
    //TODO: pass this as a uniform (from UI)
    // in range (0.0, 1.0]
    const float TINT_DELTA_DEPTH_THRESHOLD = 0.1f;
    float tintInterpolationFactor = worldSpaceDepthFragBackColour.a * (1.0f - clamp(deltaWorldSpaceDepthClamped / TINT_DELTA_DEPTH_THRESHOLD, 0.0f, 1.0f));
    vec3 tintColourAtNoon = mix(DEEP_TINT_COLOUR_AT_NOON, SHALLOW_TINT_COLOUR_AT_NOON, tintInterpolationFactor);
    // fade the tint to black when sun is lower in sky
    vec3 tintColourAtCurrentTime = clamp(sunPosition.y, 0.0f, 1.0f) * tintColourAtNoon;

    //TODO: pass this as a uniform (from UI)
    // in range [0.0, 1.0]
    const float WATER_CLARITY = 0.3f;
    vec3 waterFresnelTransmissionColour = mix(tintColourAtCurrentTime, localRefractionColour.rgb, WATER_CLARITY * localRefractionColour.a * (1.0f - deltaWorldSpaceDepthClamped));
    //TODO: fix the sun reflection highlight colour to use the global reflection colour with a similar power scalar (in order to better handle atmosphere conditions like fog/overcast/clouds)
    vec3 waterFresnelReflectionColour = mix(skybox_reflection_colour.rgb + sun_reflection_colour.rgb, localReflectionColour.rgb, localReflectionColour.a);

    // output final fragment colour...
    // reference: https://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/projgrid-hq.pdf
    //TODO: expand this later to combine other visual effects
    //TODO: currently there are visual artifacts when the camera is within the displaceable volume, but its minor and rarely noticeable
    colour = vec4(mix(waterFresnelTransmissionColour, waterFresnelReflectionColour, fresnel_f_theta), 1.0f);

    // SOFTEN THE EDGES...
    //TODO: pass this as a uniform (from UI)
    //NOTE: the lower this value, the more float imprecision is an issue and thus more artifacts
    // in range (0.0, 1.0]
    const float SOFT_EDGES_DELTA_DEPTH_THRESHOLD = 0.05f;
    float edgeHardness = 1.0f - (worldSpaceDepthFragBackColour.a * (1.0f - clamp(deltaWorldSpaceDepthClamped / SOFT_EDGES_DELTA_DEPTH_THRESHOLD, 0.0f, 1.0f)));
    //NOTE: any water covering skybox area will have hard edges (otherwise, the grid would start to fade into a circle)
    colour.a = edgeHardness;

    //TODO: in future this will be moved out into a post-process shader program
    // apply fog...
    vec4 fogColour = fogColourFarAtCurrentTime;
    fogColour.a = viewVecDepth >= fogDepthRadiusFar ? fogColour.a : viewVecDepth <= fogDepthRadiusNear ? 0.0f : fogColour.a * ((viewVecDepth - fogDepthRadiusNear) / (fogDepthRadiusFar - fogDepthRadiusNear));
    colour.rgb = mix(colour.rgb, fogColour.rgb, fogColour.a);

    //TODO: have UI toggles for these debug colours...
    //colour = heightmap_colour;
    //colour = vec4(uvViewportSpace.s, uvViewportSpace.t, uvViewportSpace.s * uvViewportSpace.t, 1.0f);
    //colour = vec4(uvViewportSpaceHeight0.s, uvViewportSpaceHeight0.t, uvViewportSpaceHeight0.s * uvViewportSpaceHeight0.t, 1.0f);
    //colour = worldSpaceDepthFragBackColour;
    //colour = vec4(worldSpaceDepthFragBackColour.a, worldSpaceDepthFragBackColour.a, worldSpaceDepthFragBackColour.a, 1.0f);
    //colour = vec4(worldSpaceDepthFragBackColour.rgb, 1.0f);
    //colour = vec4(viewVecDepthClamped, viewVecDepthClamped, viewVecDepthClamped, 1.0f);
    //colour = vec4(deltaWorldSpaceDepthClamped, deltaWorldSpaceDepthClamped, deltaWorldSpaceDepthClamped, 1.0f);
    //colour = vec4(R, 1.0f);
    //colour = skybox_reflection_colour;
    //TODO: add colour for specular highlight intensity
    //colour = sun_reflection_colour;
    //colour = vec4(fresnel_cos_theta, fresnel_cos_theta, fresnel_cos_theta, 1.0f);
    //colour = vec4(fresnel_f_theta, fresnel_f_theta, fresnel_f_theta, 1.0f);
    //colour = vec4(localReflectionsDistortionScalar, localReflectionsDistortionScalar, localReflectionsDistortionScalar, 1.0f);
    //colour = vec4(localRefractionsDistortionScalar, localRefractionsDistortionScalar, localRefractionsDistortionScalar, 1.0f);
    //colour = vec4(uvLocalReflections.s, uvLocalReflections.t, uvLocalReflections.s * uvLocalReflections.t, 1.0f);
    //colour = vec4(uvLocalRefractions.s, uvLocalRefractions.t, uvLocalRefractions.s * uvLocalRefractions.t, 1.0f);
    //colour = localReflectionColour;
    //colour = vec4(localReflectionColour.a, localReflectionColour.a, localReflectionColour.a, 1.0f);
    //colour = vec4(localReflectionColour.rgb, 1.0f);
    //colour = localRefractionColour;
    //colour = vec4(localRefractionColour.a, localRefractionColour.a, localRefractionColour.a, 1.0f);
    //colour = vec4(localRefractionColour.rgb, 1.0f);
    //colour = vec4(DEEP_TINT_COLOUR_AT_NOON, 1.0f);
    //colour = vec4(SHALLOW_TINT_COLOUR_AT_NOON, 1.0f);
    //colour = vec4(tintInterpolationFactor, tintInterpolationFactor, tintInterpolationFactor, 1.0f);
    //colour = vec4(tintColourAtNoon, 1.0f);
    //colour = vec4(tintColourAtCurrentTime, 1.0f);
    //colour = vec4(waterFresnelTransmissionColour, 1.0f);
    //colour = vec4(waterFresnelReflectionColour, 1.0f);
    //colour = vec4(edgeHardness, edgeHardness, edgeHardness, 1.0f);
    //colour = vec4(1.0f - edgeHardness, 1.0f - edgeHardness, 1.0f - edgeHardness, 1.0f);
    //colour = fogColour;
    //colour = vec4(fogColour.a, fogColour.a, fogColour.a, 1.0f);
    //colour.a = 1.0f;
}
