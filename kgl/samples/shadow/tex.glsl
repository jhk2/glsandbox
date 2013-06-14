#version 420 core

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Tex;
out vec4 out_Pos;
out vec2 out_Tex;

void main()
{
	out_Tex = in_Tex.xy;
	out_Pos = pjMatrix * mvMatrix * vec4(in_Pos, 1);
	gl_Position = out_Pos;
}
#endif

#ifdef _FRAGMENT_

in vec4 out_Pos;
in vec2 out_Tex;
out vec4 out_Color;

uniform sampler2D tex;

void main() {
        vec4 texColor = texture(tex, out_Tex);
	out_Color = texColor;
	//out_Color = vec4(1.0, 1.0, 1.0, 2.0) - texColor;
}
#endif
