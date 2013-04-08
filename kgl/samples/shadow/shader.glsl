#version 420 core

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

layout(location = 0) in vec3 in_Pos;
out vec4 out_Pos;
layout(location = 1) in vec3 in_Tex;
out vec2 out_Tex;
layout(location = 2) in vec3 in_Norm;
out vec3 out_Norm;

void main()
{
	out_Tex = in_Tex.xy;
	out_Norm = in_Norm;
	out_Pos = pjMatrix * mvMatrix * vec4(in_Pos, 1);
	gl_Position = out_Pos;
}
#endif

#ifdef _FRAGMENT_

in vec4 out_Pos;
in vec2 out_Tex;
in vec3 out_Norm;
out vec4 out_Color;

uniform layout(binding = 0) sampler2D map_Ka;

void main() {
	out_Color = texture(map_Ka, out_Tex);
	//out_Color = vec4(abs(out_Norm), 1.0);
}
#endif
