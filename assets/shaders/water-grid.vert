#version 410 core

uniform vec4 bottomLeftGridPointInWorld;
uniform vec4 bottomRightGridPointInWorld;
uniform vec3 cameraPosition;
// assuming a square grid, we get gridLength = sqrt(gridResolution) - e.g. 4x4 grid means resolution of 16 and length of 4
//NOTE: we are assuming that gridLength >= 2
uniform uint gridLength;
uniform sampler2D heightmap;
uniform vec4 topLeftGridPointInWorld;
uniform vec4 topRightGridPointInWorld;
uniform mat4 viewProjection;
uniform float waveAmplitude;

out vec4 heightmap_colour;
out vec3 normal;
out vec3 viewVec;

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
    // reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/textureSize.xhtml
    // reference: https://www.khronos.org/opengl/wiki/Sampler_(GLSL)#Texture_size_retrieval
    // reference: https://www.khronos.org/opengl/wiki/Sampler_(GLSL)#Lod_texture_access
    //NOTE: we flip the position.z, since our RH-coordinate system has +x (right) and +z (down) when staring down at XZ-plane from +y side.
    // by doing this and dividing by the texture dimensions, we can map each texel colour to each unit of space on the XZ-plane with the same orientation as the image
    //NOTE: setting the LOD value at 0 (base mipmap)
    ivec2 heightmap_resolution = textureSize(heightmap, 0);
    float heightmap_hres = heightmap_resolution.x;
    float heightmap_vres = heightmap_resolution.y;
    //TODO: add a scalar uniform that can control the world-space to texel-space mapping (right now it should implicitly be one-to-one)
    return texture(heightmap, vec2(position.x / heightmap_hres, -position.z / heightmap_vres));
}

//TODO: animate this with a passed in uniform (can use sin() oscillation)
float computeYPosition(in vec4 heightmapSample) {
    float heightmap_intensity_0_to_1 = heightmapSample.r;
    float heightmap_intensity_neg1_to_1 = 2.0f * (heightmap_intensity_0_to_1 - 0.5f);
    return waveAmplitude * heightmap_intensity_neg1_to_1;
}

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

    // compute the interpolated world-space position for this vertexID...
    vec4 position = computeInterpolatedGridPosition(uv);

    // output a debug colour corresponding to the sampled heightmap colour
    heightmap_colour = sampleHeightmap(position);

    // displace this grid vertex along the y-axis based on the height data
    position.y = computeYPosition(heightmap_colour);

    // output final vertex position in clip-space
    gl_Position = viewProjection * position;

    // output view vector (V)
    //NOTE: this should always be defined as a unit vector pointing away from a surface point towards the camera eye
    viewVec = normalize(cameraPosition - position.xyz);

    //TODO: if possible, it would be nice to get all the vertex positions computed and then pass them off to another shader stage to compute all the normals without redundant calculation, but this works for now
    //      maybe you could render the positions to a texture and then sample the neighbours, but that is overly complicated and may even be slower
    // compute four adjacent vertex positions...
    vec4 pos_minus_du = computeInterpolatedGridPosition(uv - vec2(du, 0.0f));
    pos_minus_du.y = computeYPosition(sampleHeightmap(pos_minus_du));
    vec4 pos_plus_du = computeInterpolatedGridPosition(uv + vec2(du, 0.0f));
    pos_plus_du.y = computeYPosition(sampleHeightmap(pos_plus_du));
    vec4 pos_minus_dv = computeInterpolatedGridPosition(uv - vec2(0.0f, dv));
    pos_minus_dv.y = computeYPosition(sampleHeightmap(pos_minus_dv));
    vec4 pos_plus_dv = computeInterpolatedGridPosition(uv + vec2(0.0f, dv));
    pos_plus_dv.y = computeYPosition(sampleHeightmap(pos_plus_dv));

    //NOTE: since the projector is currently always above the water, this original normal will always be pointing upwards (+y)
    normal = normalize(cross((pos_plus_du - pos_minus_du).xyz, (pos_plus_dv - pos_minus_dv).xyz));
    // flip the normal when the camera is underwater...
    if (dot(normal, viewVec) < 0.0f) normal *= -1;
}
