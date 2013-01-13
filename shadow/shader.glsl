#version 420 core

#ifdef _VERTEX_

in vec4 in_Pos;
out vec4 out_Pos;

void main()
{
	gl_Position = vec4(0, 0, 0, 1);
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
	out_Color = vec4(1.0, gaussian(length(out_Pos)*4), 0, 1.0);
}
#endif
