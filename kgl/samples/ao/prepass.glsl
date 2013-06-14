#version 430 core

#ifdef _VERTEX_

uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

in layout(location = 0) vec3 in_Pos;

void main()
{
    gl_Position = pjMatrix * mvMatrix * vec4(in_Pos, 1);
}

#endif

#ifdef _FRAGMENT_

void main()
{

}

#endif
