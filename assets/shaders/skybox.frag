#version 410 core

// used as a grayscale intensity threshold acting as a way to control the proportion of clouds from the skybox textures get drawn
uniform float oneMinusCloudProportion;
// used to scale both "fog/blur" of atmosphere and increase grayness of clouds
uniform float overcastStrength;
uniform samplerCube skyboxClouds;
// the inverse light direction
uniform vec3 sunPosition;

in vec3 STR;

out vec4 colour;

void main() {
    vec3 rawColourRGB = texture(skyboxClouds, STR).rgb;
    // convert to grayscale...
    // reference: https://stackoverflow.com/questions/17615963/standard-rgb-to-grayscale-conversion
    // reference: https://en.wikipedia.org/wiki/Grayscale#Converting_color_to_grayscale
    //TODO: a comment in the SO link mentions that there might be another step to this, but this seems alright
    float C_linear = 0.2126f * rawColourRGB.r + 0.7152f * rawColourRGB.g + 0.0722f * rawColourRGB.b;
    float C_srgb = C_linear <= 0.0031308f ? 12.92f * C_linear : 1.055f * pow(C_linear, 1.0f / 2.4f) - 0.055f;
    vec4 grayscaleColour = vec4(C_srgb, C_srgb, C_srgb, 1.0f);

    const float MIN_CLOUD_INTENSITY = 0.2f;
    float cloudIntensity = clamp(sunPosition.y, MIN_CLOUD_INTENSITY, 1.0f);
    vec3 cloudColour = vec3(cloudIntensity, cloudIntensity, cloudIntensity);

    // only draw cloud (set alpha > 0.0) if it meets the proportion threshold
    // NOTE: (C_srgb - oneMinusCloudProportion) / (1.0f - oneMinusCloudProportion) =:= scaling operation to map all the threshold-meeting C_srgb intensities to range [0.0, 1.0]
    colour = vec4(cloudColour, C_srgb > oneMinusCloudProportion ? (C_srgb - oneMinusCloudProportion) / (1.0f - oneMinusCloudProportion) : 0.0f);

    float a = clamp(sunPosition.y, 0.0f, 1.0f) * overcastStrength;
    const vec3 FOG_COLOUR = vec3(0.5f, 0.5f, 0.5f);

    colour = C_srgb > oneMinusCloudProportion ? mix(colour, grayscaleColour, a) : vec4(FOG_COLOUR, a);

    // debug colours...
    // grayscale
    //colour = grayscaleColour;
    // texture coordinate visualization
    //colour = vec4(STR, 1.0f);
}
