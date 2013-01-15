#version 420 core

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

layout(location = 0) in vec3 in_Pos;
out vec4 out_Pos;

void main()
{
	out_Pos = pjMatrix * mvMatrix * vec4(in_Pos, 1);
	gl_Position = out_Pos;
	//gl_Position = vec4(in_Pos, 1);
	//gl_Position = vec4(0, 0, 0, 1);
}
#endif

#ifdef _FRAGMENT_

in vec4 out_Pos;
out vec4 out_Color;

// gaussian function
#define M_PI 3.1415926535897932384626433832795
#define SIGMA 0.84
float gaussian(float x) {
	return (1/sqrt(2*M_PI*SIGMA*SIGMA))*exp(-(x*x)/(2*SIGMA*SIGMA));
}

void main() {
	//out_Color = vec4(1.0, gaussian(length(out_Pos)*4), 0, 1.0);
	out_Color = vec4(1, out_Pos.x, out_Pos.y, 1);
	//out_Color = vec4(0, 1, 0, 1);
	//out_Color = vec4(0);
}
#endif
