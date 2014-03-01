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
#define TESSLEVEL 4

void main() {
    tc_Pos[gl_InvocationID] = v_Pos[gl_InvocationID];
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = TESSLEVEL;
        gl_TessLevelOuter[0] = TESSLEVEL;
        gl_TessLevelOuter[1] = TESSLEVEL;
        gl_TessLevelOuter[2] = TESSLEVEL;
    }
}
#endif // _TESSCONTROL_

#ifdef _TESSEVAL_
uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

layout(triangles, equal_spacing, ccw) in;
in vec3 tc_Pos[];
out vec3 te_Pos;
void main() {
    vec3 p0 = gl_TessCoord.x * tc_Pos[0];
    vec3 p1 = gl_TessCoord.y * tc_Pos[1];
    vec3 p2 = gl_TessCoord.z * tc_Pos[2];
    te_Pos = normalize(p0 + p1 + p2);
    gl_Position = pjMatrix * mvMatrix * vec4(te_Pos, 1);
}
#endif // _TESSEVAL_

#ifdef _GEOMETRY_
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
in vec3 te_Pos[3];
out vec3 g_BaryCoord;

void main() {
    g_BaryCoord = vec3(1,0,0);
    gl_Position = gl_in[0].gl_Position; EmitVertex();
    g_BaryCoord = vec3(0,1,0);
    gl_Position = gl_in[1].gl_Position; EmitVertex();
    g_BaryCoord = vec3(0,0,1);
    gl_Position = gl_in[2].gl_Position; EmitVertex();
    EndPrimitive();
}
#endif // _GEOMETRY_

#ifdef _FRAGMENT_
uniform layout(location = 2) float selected;
in vec3 g_BaryCoord;
out layout(location = 0) vec4 out_Color;

// wireframe generation code from http://prideout.net/blog/?p=48
float border(float d, float scale, float offset) {
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

void main() {
    float d = min(min(g_BaryCoord.x, g_BaryCoord.y), g_BaryCoord.z);
    out_Color = border(d, 40, -0.5) * mix(vec4(1.0), vec4(0, 1.0, 0, 1.0), selected);
}

#endif // _FRAGMENT_
