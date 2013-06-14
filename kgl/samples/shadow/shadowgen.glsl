#version 420 core

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

layout(location = 0) in vec3 in_Pos;

void main()
{
	gl_Position = pjMatrix * mvMatrix * vec4(in_Pos, 1);
}
#endif

#ifdef _FRAGMENT_

//out vec4 out_Color;
//#define ESM_K 30

void main() {
        //out_Color = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}
#endif
