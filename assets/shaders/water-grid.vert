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

void main() {
    // example of what the expected vertexID layout is (using a length = 4 grid for demonstration)...
    //
    // 12 13 14 15
    //  8  9 10 11
    //  4  5  6  7
    //  0  1  2  3

    //NOTE: using integer division here to our advantage
    vec2 uv = vec2(mod(gl_VertexID, gridLength), gl_VertexID / gridLength) / float(gridLength - 1);

    // this makes our grid into a bunch of (u, v) for interpolation of grid corner positions in world space...
    //
    // (0.0f, 1.0f) ... ... (1.0f, 1.0f)
    //      |                    |
    //      |                    |
    // (0.0f, 0.0f) ... ... (1.0f, 0.0f) 

    // compute the interpolated world-space position for this vertexID...
    vec4 mix_u_1 = mix(bottomLeftGridPointInWorld, bottomRightGridPointInWorld, uv.s);
    vec4 mix_u_2 = mix(topLeftGridPointInWorld, topRightGridPointInWorld, uv.s);
    vec4 position = mix(mix_u_1, mix_u_2, uv.t);

    //TODO: do we have to handle overflow? (either here or in C++ program)?
    //TODO: I think it would be best to actually use the texture as 8-bit since that is what the image is. Right now all textures are used as RGBA.
    //TODO: animate this with a passed in uniform (can use sin() oscillation)
    // reference: https://open.gl/textures
    //NOTE: the heightmap is assumed to be either setup with wrapping as GL_REPEAT or GL_MIRRORED_REPEAT

    // output a debug colour corresponding to the sampled heightmap colour
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
    heightmap_colour = texture(heightmap, vec2(position.x / heightmap_hres, -position.z / heightmap_vres));

    float heightmap_intensity_0_to_1 = heightmap_colour.r;
    float heightmap_intensity_neg1_to_1 = 2.0f * (heightmap_intensity_0_to_1 - 0.5f);
    position.y = waveAmplitude * heightmap_intensity_neg1_to_1;

    // output final vertex position in clip-space
    gl_Position = viewProjection * position;

    //TODO: compute the actual normal for this vertex (from the heightmap) and pass it out
    //NOTE: just assuming a perfect mirror for now
    normal = vec3(0.0f, 1.0f, 0.0f);

    // output view vector (V)
    //NOTE: this should always be defined as a unit vector pointing away from a surface point towards the camera eye
    viewVec = normalize(cameraPosition - position.xyz);
}
