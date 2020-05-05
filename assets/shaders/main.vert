#version 410 core

// default "plane" is a singularity which will cause this manual clipping test to always succeed for all vertices
//NOTE: this symbolic value should always be passed when you want this manual clipping disabled (cause some drivers might ignore glEnable/glDisable of GL_CLIP_DISTANCEi)
//NOTE: if you ever output a clip distance that isn't enabled, the clipping stage will just ignore the manual test
// reference: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_ClipDistance.xhtml
// reference: https://prideout.net/clip-planes
uniform vec4 clipPlane0 = vec4(0.0f, 0.0f, 0.0f, 1.0f); // <A, B, C, D> where Ax + By + Cz = D
uniform mat4 modelMat;
uniform mat4 modelViewMat;
uniform mat4 mvpMat;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 colour;

out vec3 COLOUR;
out vec3 normalVec;
out vec2 UV;
out vec3 viewSpacePosition;
out vec3 viewVec;

out float gl_ClipDistance[1];

void main() {
    vec4 positionHomogenous = vec4(position, 1.0f);
    vec4 normalHomogenous = vec4(normal, 0.0f);

    // output (pass-throughs)...
    COLOUR = colour;
    UV = uv;

    // output view-space position...
    viewSpacePosition = (modelViewMat * positionHomogenous).xyz;
    // output view vector...
    viewVec = normalize(viewSpacePosition);

    // output clip-space position...
    gl_Position = mvpMat * positionHomogenous;

    // apply manual clip plane...
    gl_ClipDistance[0] = dot(modelMat * positionHomogenous, clipPlane0);

    // output normal vector
    normalVec = normalize((modelViewMat * normalHomogenous).xyz);
}
