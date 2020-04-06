#version 410 core

// this is the sky-gradient texture that will be interpolated based on time of day
uniform sampler1D skysphere;
// this scales the sun horizon colour to be darker
uniform float sunHorizonDarkness;
// the inverse light direction
uniform vec3 sunPosition;
// higher shininess means smaller specular highlight (sun)
uniform float sunShininess;
// this scalar affects how much of the sun light is added on top of the diffuse sky colour
uniform float sunStrength;

in vec3 worldPosition;

out vec4 colour;

void main() {
    float a_1 = 0.25f * (sunPosition.y + 1.0f);
    float a_2 = 1.0f - a_1;
    vec3 sunPeakColour = texture(skysphere, a_1).rgb;
    vec3 sunHorizonColour = mix(texture(skysphere, a_2).rgb, vec3(0.0f, 0.0f, 0.0f), sunHorizonDarkness);

    float a_3 = dot(worldPosition, sunPosition);
    vec3 diffuseColour = a_3 >= 0.0f ? mix(sunHorizonColour, sunPeakColour, a_3) : mix(sunHorizonColour, vec3(0.0f, 0.0f, 0.0f), abs(a_3) * (1.0f - abs(sunPosition.y)));

    // the sun is drawn as a specular hightlight that will be same (or similar) as the reflection on the water
    const vec3 SUN_BASE_COLOUR = vec3(1.0f, 1.0f, 1.0f);
    vec3 specularColour = a_3 > 0.0f ? sunStrength * SUN_BASE_COLOUR * pow(a_3, sunShininess) : vec3(0.0f, 0.0f, 0.0f);

    // when sunPosition.y >= this, then this fragment will be fully opaque. Thus this should be in range [-1.0, 1.0]
    //NOTE: this is used to make sure no stars are seen during the day
    const float FULLY_OPAQUE_THRESHOLD = 0.1f;
    const float FULLY_OPAQUE_SHIFT = clamp(1.0f - FULLY_OPAQUE_THRESHOLD, 0.0f, 2.0f);
    //NOTE: (sunPosition.y + 1.0f) / (FULLY_OPAQUE_THRESHOLD + 1.0f) is a mapping to the range [0.0, 1.0) where it interpolates from sunPosition.y == -1.0 (inclusive) upto sunPosition.y == FULLY_OPAQUE_THRESHOLD (exclusive)
    float skysphereAlpha_1 = sunPosition.y < FULLY_OPAQUE_THRESHOLD ? (sunPosition.y + 1.0f) / (FULLY_OPAQUE_THRESHOLD + 1.0f) : 1.0f;
    //NOTE: this second factor is used to scale stars to be duller when close to sun position
    float skysphereAlpha_2 = (a_3 + 1.0f) / 2.0f;
    float skysphereAlpha = max(skysphereAlpha_1, skysphereAlpha_2);
    //TODO: could add a clamped lower bound in order to simulate nightime overcast/fog, but everything looks good regardless
    colour = vec4(diffuseColour + specularColour, skysphereAlpha);

    // debug colours...
    // texture coordinate visualization
    //colour = vec4(worldPosition, 1.0f);
}
