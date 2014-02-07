#version 430 core

#ifdef _VERTEX_

uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

in layout(location = 0) vec3 in_Pos;
in layout(location = 1) vec3 in_Tex;
in layout(location = 2) vec3 in_Norm;

out vec2 out_Tex;
out vec3 out_Norm;

void main() {
    out_Tex = in_Tex.st;

    mat3 upper = mat3(mvMatrix);
    mat3 normalMatrix = transpose(inverse(upper));
    out_Norm = normalMatrix * in_Norm; // view space normal

    vec4 mvPos = mvMatrix * vec4(in_Pos, 1);
    gl_Position = pjMatrix * mvPos;
}

#endif // _VERTEX_

#ifdef _FRAGMENT_

uniform ObjMaterial {
        float Ns;
        float Ni;
        float Tr;
        vec3 Tf;
        unsigned int illum;
        vec3 Ka;
        vec3 Kd;
        vec3 Ks;
        vec3 Ke;
};

uniform layout(binding = 0) sampler2D map_Ka;
uniform layout(binding = 1) sampler2D map_Kd;
uniform layout(binding = 2) sampler2D map_Ks;

in vec2 out_Tex;
in vec3 out_Norm;

out layout(location = 0) vec3 ambient_Color;
out layout(location = 1) vec3 diffuse_Color;
out layout(location = 2) vec4 specular_Color;
out layout(location = 3) vec3 view_Normal;

void main() {
    ambient_Color = texture(map_Ka, out_Tex).rgb;
    diffuse_Color = texture(map_Kd, out_Tex).rgb;
    specular_Color = vec4(texture(map_Ks, out_Tex).rgb, Ns);
    view_Normal = out_Norm;
}

#endif // _FRAGMENT_
