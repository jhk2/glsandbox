#version 420 core

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

layout(location = 0) in vec3 in_Pos;

void main()
{
    vec4 world_Pos = mvMatrix * vec4(in_Pos, 1);
    gl_Position = pjMatrix * world_Pos;
}
#endif//_VERTEX_

#ifdef _FRAGMENT_
out vec4 out_Color;
#define ESM_C 80

#define NEAR 20.0f
#define FAR 100.f

float getLinearDepth(float depth) {
    return (2 * NEAR) / (FAR + NEAR - depth * (FAR - NEAR));
}

void main() {
    // nonlinear depth
    //out_Color = vec4(exp(gl_FragCoord.z * ESM_C), 0, 0, 1.0);
    // linear depth
    float linDepth = getLinearDepth(gl_FragCoord.z);
    out_Color = vec4(exp(linDepth * ESM_C), 0, 0, 1.0);
}
#endif//_FRAGMENT_
