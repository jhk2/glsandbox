#version 420 core

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

layout(location = 0) in vec3 in_Pos;
layout(location = 3) in vec3 in_Color;
out vec4 out_Pos;
out vec3 vert_Color;

void main()
{
	vert_Color = in_Color;
	out_Pos = pjMatrix * mvMatrix * vec4(in_Pos, 1);
	gl_Position = out_Pos;
}
#endif

#ifdef _FRAGMENT_

in vec4 out_Pos;
in vec3 vert_Color;
out vec4 out_Color;

void main() {
	out_Color = vec4(vert_Color, 1.0);
}
#endif
