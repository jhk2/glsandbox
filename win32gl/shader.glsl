#ifdef _VERTEX_
void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
#endif

#ifdef _FRAGMENT_
void main()
{
	gl_FragColor = vec4(0, 1.0, 0, 1.0);
}
#endif
