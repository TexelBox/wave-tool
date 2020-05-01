#version 410 core

//NOTE: this must match the value in the C++ program
const uint MAX_COUNT_OF_GERSTNER_WAVES = 4;

uniform vec4 bottomLeftGridPointInWorld;
uniform vec4 bottomRightGridPointInWorld;
uniform vec3 cameraPosition;

// reference: https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
// reference: https://github.com/CaffeineViking/osgw/blob/master/share/shaders/gerstner.glsl
// caller must pass in the number of initialized waves
uniform uint gerstnerWaveCount = 0;
uniform struct GerstnerWave {
    float amplitude_A;
    float frequency_w;
    float phaseConstant_phi;
    float steepness_Q_i;
    vec2 xzDirection_D;
} gerstnerWaves[MAX_COUNT_OF_GERSTNER_WAVES];

// assuming a square grid, we get gridLength = sqrt(gridResolution) - e.g. 4x4 grid means resolution of 16 and length of 4
//NOTE: we are assuming that gridLength >= 2
uniform uint gridLength;
uniform sampler2D heightmap;
uniform float heightmapDisplacementScale; // in range [0.0, inf)
uniform float heightmapSampleScale; // in range [0.0, inf)
uniform vec4 topLeftGridPointInWorld;
uniform vec4 topRightGridPointInWorld;
uniform float verticalBounceWaveDisplacement;
uniform mat4 viewProjection;
uniform float waveAnimationTimeInSeconds; // in range [0.0, inf)
uniform float zFar;

out vec4 heightmap_colour;
out vec3 normal;
out vec3 viewVec;
out float viewVecDepth;

// reference: https://developer.nvidia.com/gpugems/gpugems/part-i-natural-effects/chapter-1-effective-water-simulation-physical-models
//TODO: it seems like the direction is interpreted backwards?
vec3 computeGerstnerSurfacePosition(in vec2 xzGridPosition, in float timeInSeconds) {
    vec3 gerstnerSurfacePosition = vec3(xzGridPosition.x, 0.0f, xzGridPosition.y);
    for (uint i = 0; i < gerstnerWaveCount; ++i) {
        // this contribution of this wave...
        float xyzConstant_1 = gerstnerWaves[i].frequency_w * dot(gerstnerWaves[i].xzDirection_D, xzGridPosition) + gerstnerWaves[i].phaseConstant_phi * timeInSeconds;
        float xzConstant_1 = gerstnerWaves[i].steepness_Q_i * cos(xyzConstant_1);
        vec3 gerstnerWavePosition = gerstnerWaves[i].amplitude_A * vec3(gerstnerWaves[i].xzDirection_D.x * xzConstant_1, sin(xyzConstant_1), gerstnerWaves[i].xzDirection_D.y * xzConstant_1);
        gerstnerSurfacePosition += gerstnerWavePosition;
    }

    return gerstnerSurfacePosition;
}

vec4 computeInterpolatedGridPosition(in vec2 uv) {
    vec4 mix_u_1 = mix(bottomLeftGridPointInWorld, bottomRightGridPointInWorld, uv.s);
    vec4 mix_u_2 = mix(topLeftGridPointInWorld, topRightGridPointInWorld, uv.s);
    return mix(mix_u_1, mix_u_2, uv.t);
}

vec4 sampleHeightmap(in vec4 position) {
    //TODO: do we have to handle overflow? (either here or in C++ program)?
    //TODO: I think it would be best to actually use the texture as 8-bit since that is what the image is. Right now all textures are used as RGBA.
    // reference: https://open.gl/textures
    //NOTE: the heightmap is assumed to be either setup with wrapping as GL_REPEAT or GL_MIRRORED_REPEAT
    //NOTE: increase the scale for "rougher water"
    //NOTE: the scale =:= proportion of texel row/column that fits in 1 unit along the respective axis
    // e.g. a scale of 1.0 means the 2D texture is entirely fit inside a 1 unit x 1 unit cell of world space
    // e.g. a scale of 0.1 means the 2D texture is entirely fit inside a 10 unit x 10 unit cell of world space
    //NOTE: for an intuitive mapping-orientation of the texture, we flip the position.z, since our RH-coordinate system has +x (right) and +z (down) when staring down at XZ-plane from +y side.
    return texture(heightmap, heightmapSampleScale * vec2(position.x, -position.z));
}

float computeHeightmapDisplacement(in vec4 heightmapSample) {
    float heightmap_intensity_0_to_1 = heightmapSample.r;
    float heightmap_intensity_neg1_to_1 = 2.0f * (heightmap_intensity_0_to_1 - 0.5f);
    //NOTE: increase the scale for "steeper water"
    return heightmapDisplacementScale * heightmap_intensity_neg1_to_1;
}

//TODO: animate the heightmap displacement values over time + increase randomness (reduce tiling visuals)
void main() {
    // example of what the expected vertexID layout is (using a length = 4 grid for demonstration)...
    //
    // 12 13 14 15
    //  8  9 10 11
    //  4  5  6  7
    //  0  1  2  3

    //NOTE: using integer division here to our advantage
    float du = 1.0f / (gridLength - 1);
    float dv = 1.0f / (gridLength - 1);
    vec2 uv = vec2(mod(gl_VertexID, gridLength) * du, (gl_VertexID / gridLength) * dv);

    // this makes our grid into a bunch of (u, v) for interpolation of grid corner positions in world space...
    //
    // (0.0f, 1.0f) ... ... (1.0f, 1.0f)
    //      |                    |
    //      |                    |
    // (0.0f, 0.0f) ... ... (1.0f, 0.0f) 

    // compute the interpolated world-space grid position for this vertexID...
    vec4 position = computeInterpolatedGridPosition(uv);

    // apply gerstner...
    position = vec4(computeGerstnerSurfacePosition(position.xz, waveAnimationTimeInSeconds), 1.0f);

    // output a debug colour corresponding to the sampled heightmap colour
    heightmap_colour = sampleHeightmap(position);

    // displacement bumps...
    position.y += computeHeightmapDisplacement(heightmap_colour);

    // vertical bounce...
    position.y += verticalBounceWaveDisplacement;

    // output final vertex position in clip-space
    gl_Position = viewProjection * position;

    // output view depth and view vector (V)
    //NOTE: this should always be defined as a unit vector pointing away from a surface point towards the camera eye
    vec3 viewVecRaw = cameraPosition - position.xyz;
    float viewVecLength = length(viewVecRaw);
    viewVec = viewVecRaw / viewVecLength;
    //TODO: clipping will probably interpolate wrong if I clamp the upper bound, so just remove this clamping when I implement the spherical clipping
    viewVecDepth = clamp(viewVecLength / zFar, 0.0f, 1.0f);

    //TODO: refactor all these position calculations into 1 function
    //TODO: if possible, it would be nice to get all the vertex positions computed and then pass them off to another shader stage to compute all the normals without redundant calculation, but this works for now
    //      maybe you could render the positions to a texture and then sample the neighbours, but that is overly complicated and may even be slower
    // compute four adjacent vertex positions...
    vec4 pos_minus_du = computeInterpolatedGridPosition(uv - vec2(du, 0.0f));
    vec4 pos_plus_du = computeInterpolatedGridPosition(uv + vec2(du, 0.0f));
    vec4 pos_minus_dv = computeInterpolatedGridPosition(uv - vec2(0.0f, dv));
    vec4 pos_plus_dv = computeInterpolatedGridPosition(uv + vec2(0.0f, dv));

    pos_minus_du = vec4(computeGerstnerSurfacePosition(pos_minus_du.xz, waveAnimationTimeInSeconds), 1.0f);
    pos_plus_du = vec4(computeGerstnerSurfacePosition(pos_plus_du.xz, waveAnimationTimeInSeconds), 1.0f);
    pos_minus_dv = vec4(computeGerstnerSurfacePosition(pos_minus_dv.xz, waveAnimationTimeInSeconds), 1.0f);
    pos_plus_dv = vec4(computeGerstnerSurfacePosition(pos_plus_dv.xz, waveAnimationTimeInSeconds), 1.0f);

    pos_minus_du.y += computeHeightmapDisplacement(sampleHeightmap(pos_minus_du));
    pos_plus_du.y += computeHeightmapDisplacement(sampleHeightmap(pos_plus_du));
    pos_minus_dv.y += computeHeightmapDisplacement(sampleHeightmap(pos_minus_dv));
    pos_plus_dv.y += computeHeightmapDisplacement(sampleHeightmap(pos_plus_dv));

    pos_minus_du.y += verticalBounceWaveDisplacement;
    pos_plus_du.y += verticalBounceWaveDisplacement;
    pos_minus_dv.y += verticalBounceWaveDisplacement;
    pos_plus_dv.y += verticalBounceWaveDisplacement;

    //NOTE: since the projector is currently always above the water and our waves have no y-overlaps, this original normal will always be pointing upwards (+y)
    //TODO: probably check this assumption to be safe
    //TODO: currently some of the normals appear black (0 vector or negative?) when debug rendered as the fragment colour, so fix this bug (could be from flipping or bad math)
    normal = normalize(cross((pos_plus_du - pos_minus_du).xyz, (pos_plus_dv - pos_minus_dv).xyz));
    // flip the normal when the camera is underwater...
    //TODO: I think this is inaccurate when camera is close to water surface, so a fix would be to compute the "water position.y" where the camera is (from camera.xz) and then compare water position.y to cameraPosition.y to decide if flipping is needed
    if (dot(normal, viewVec) < 0.0f) normal *= -1;
}
