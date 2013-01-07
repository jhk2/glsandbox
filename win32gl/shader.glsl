#ifdef _VERTEX_
void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
#endif

#ifdef _FRAGMENT_
precision mediump float;
varying vec2 out_Pos;
uniform vec2 touchPoint;
uniform float strength;

// gaussian function
#define M_PI 3.1415926535897932384626433832795
#define SIGMA 0.84
float gaussian(float x) {
	return (1/sqrt(2*M_PI*SIGMA*SIGMA))*exp(-(x*x)/(2*SIGMA*SIGMA));
}

void main() {
	gl_FragColor = vec4(1.0, gaussian(out_Pos*4), 0, 1.0);
}
#endif
