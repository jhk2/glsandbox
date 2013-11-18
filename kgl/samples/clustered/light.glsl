#version 430 core

#ifdef _VERTEX_

in layout(location = 0) vec3 in_Pos;
out vec3 v_Pos;

void main() {
    v_Pos = in_Pos;
    //gl_Position = pjMatrix * mvMatrix * vec4(in_Pos, 1.0);
}

#endif // _VERTEX_

#ifdef _TESSCONTROL_
layout(vertices = 3) out;
in vec3 v_Pos[];
out vec3 tc_Pos[];

void main() {
    tc_Pos[gl_InvocationID] = v_Pos[gl_InvocationID];
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = 16;
        gl_TessLevelOuter[0] = 16;
        gl_TessLevelOuter[1] = 16;
        gl_TessLevelOuter[2] = 16;
    }
}
#endif // _TESSCONTROL_

#ifdef _TESSEVAL_
uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

layout(triangles, equal_spacing, cw) in;
in vec3 tc_Pos[];
out vec3 te_Pos;
void main() {
    vec3 p0 = gl_TessCoord.x * tc_Pos[0];
    vec3 p1 = gl_TessCoord.y * tc_Pos[1];
    vec3 p2 = gl_TessCoord.z * tc_Pos[2];
    te_Pos = normalize(p0 + p1 + p2);//length(tcPosition[0]);
    gl_Position = pjMatrix * mvMatrix * vec4(te_Pos, 1);
}
#endif // _TESSEVAL_

#ifdef _FRAGMENT_

out layout(location = 0) vec4 out_Color;

void main() {
    out_Color = vec4(1.0);
}

#endif // _FRAGMENT_
