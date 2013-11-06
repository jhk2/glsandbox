#version 430 core

#ifdef _VERTEX_

uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

in layout(location = 0) vec3 in_Pos;
in layout(location = 1) vec3 in_Tex;

out vec2 out_Tex;

void main() {
    out_Tex = in_Tex.st;
    gl_Position = pjMatrix * mvMatrix * vec4(in_Pos, 1.0);
}

#endif // _VERTEX_

#ifdef _FRAGMENT_

uniform layout(binding = 0) sampler2D ambient_map;
uniform layout(binding = 1) sampler2D diffuse_map;
uniform layout(binding = 2) sampler2D specular_map;
uniform layout(binding = 3) sampler2D normal_map;

in vec2 out_Tex;

out layout(location = 0) vec4 out_Color;

void main() {
    vec4 ambient_Color = texture(ambient_map, out_Tex);
    vec4 diffuse_Color = texture(diffuse_map, out_Tex);
    vec4 specular_Color = texture(specular_map, out_Tex);
    vec3 view_Normal = texture(normal_map, out_Tex).xyz;
    out_Color = vec4(view_Normal, 1.0);
}

#endif // _FRAGMENT_
