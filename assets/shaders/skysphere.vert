#version 410 core

uniform mat4 VPNoTranslation;

layout (location = 0) in vec3 vertex;

out vec3 normalVec;

void main() {
    // normalize the vertex position to ensure we have a unit sphere
    vec3 vertexNormalized = normalize(vertex);

    // output position in clip-space
    gl_Position = VPNoTranslation * vec4(vertexNormalized, 1.0f);
    // output normal vector
    normalVec = vertexNormalized;
}
