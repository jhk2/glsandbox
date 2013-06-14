#version 420 core

uniform mat4 lightmv;

#ifdef _VERTEX_

uniform mat4 mvMatrix;
uniform mat4 pjMatrix;

uniform mat4 lightpj;
uniform vec3 light_Pos;

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec3 in_Tex;
layout(location = 2) in vec3 in_Norm;
out vec4 out_Pos;
out vec2 out_Tex;
out vec3 out_Norm;
out vec4 light_Tex;
out vec4 lightPos;

const mat4 biasMatrix = mat4(
        vec4(0.5,0,0,0),
        vec4(0,0.5,0,0),
        vec4(0,0,0.5,0),
        vec4(0.5,0.5,0.5,1.0));

void main()
{
    out_Tex = in_Tex.xy;

    // position of object in camera space
    out_Pos = mvMatrix * vec4(in_Pos, 1);

    gl_Position = pjMatrix * out_Pos;

    // position in light space
    light_Tex = biasMatrix * lightpj * lightmv * vec4(in_Pos, 1);
    // position of light in camera space
    lightPos = mvMatrix * vec4(light_Pos, 1);

    // for normal matrix, take upper 3x3 of modelview
    mat3 upper = mat3(mvMatrix);
    // invert and transpose
    mat3 normalMatrix = transpose(inverse(upper));
    out_Norm = normalMatrix * in_Norm;
}

#endif

#ifdef _FRAGMENT_

in vec4 out_Pos;
in vec2 out_Tex;
in vec3 out_Norm;
in vec4 light_Tex;
in vec4 lightPos;
out vec4 out_Color;

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

const float Ka_ = 0.2;
const float Kd_ = 0.8;

uniform sampler2D map_Ka;
uniform sampler2D map_Kd;
uniform sampler2DShadow shadow;
uniform sampler2D shadowTex;

#define ESM_K 30
#define OVERDARK 0.5

float esm(vec3 lightTex) {
    float d = lightTex.z; // position of object in light space
    float expz = textureLod(shadowTex, lightTex.xy, 4.0).r;// exponential shadowmap value (filtered)
    return clamp(exp(-ESM_K * (d - expz)), 0.0, 1.0);
}

void main() {
    vec3 lTex = light_Tex.xyz / light_Tex.w;

    bvec2 outside = greaterThan(lTex.xy,vec2(1.0,1.0));
    bvec2 inside = lessThan(lTex.xy,vec2(0,0));

    float kshadow = 0;

    if (all(not(inside)) && all(not(outside))) {
            kshadow = esm(lTex);
    }

    // vector from object to light
    vec4 objtolight = lightPos - out_Pos;
    objtolight = normalize(objtolight);

    // diffuse lighting is n dot l
    vec4 diffuseTex = texture(map_Kd, out_Tex);
    float ndotl = max(dot(objtolight.xyz, out_Norm), 0);
    vec3 diffuse = ndotl * Kd_ * kshadow * diffuseTex.xyz;

    vec4 ambientTex = texture(map_Ka, out_Tex);
    vec3 ambient = Ka_ * ambientTex.xyz;

    out_Color = vec4(diffuse + ambient, min(1.0, diffuseTex.w + ambientTex.w));
}


#endif
