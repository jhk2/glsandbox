#version 430 core

#ifdef _VERTEX_

uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

in layout(location = 0) vec3 in_Pos;
in layout(location = 2) vec3 in_Norm;
out vec3 out_Pos;
out vec3 out_Norm;

void main()
{
    mat3 upper = mat3(mvMatrix);
    mat3 normalMatrix = transpose(inverse(upper));
    out_Norm = normalMatrix * in_Norm; // view space normal

    vec4 mvPos = mvMatrix * vec4(in_Pos, 1);
    out_Pos = mvPos.xyz; // view space position
    gl_Position = pjMatrix * mvPos;
}

#endif

#ifdef _FRAGMENT_
in vec3 out_Pos;
in vec3 out_Norm;
out layout(location = 0) vec3 view_Pos;
out layout(location = 1) vec3 view_Norm;
void main()
{
    view_Pos = out_Pos;
    view_Norm = out_Norm;
}

#endif
