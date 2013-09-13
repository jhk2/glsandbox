#version 420 core

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

layout(location = 0) in vec3 in_Pos;

void main()
{
    gl_Position = pjMatrix * mvMatrix * vec4(in_Pos, 1);
}
#endif//_VERTEX_

#ifdef _FRAGMENT_
out vec4 out_Color;
#define ESM_C 30
void main() {
    out_Color = vec4(exp(gl_FragCoord.z*ESM_C), 0, 0, 1.0);
}
#endif//_FRAGMENT_
