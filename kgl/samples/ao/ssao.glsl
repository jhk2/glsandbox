#version 430 core

uniform layout(location = 2) mat4 eyePj; // perspective camera used to render original scene

#ifdef _VERTEX_
uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

in layout(location = 0) vec3 in_Pos;
in layout(location = 1) vec3 in_Tex;

out vec2 out_Tex;

void main()
{
    out_Tex = in_Tex.st;
    gl_Position = pjMatrix * mvMatrix * vec4(in_Pos, 1.0);
}

#endif

#ifdef _FRAGMENT_

uniform layout(binding = 0) sampler2D positionMap;
uniform layout(binding = 1) sampler2D normalMap;

in vec2 out_Tex;

out vec4 out_Color;

const vec3 taps[16] = { vec3(-0.364452, -0.014985, -0.513535),
                        vec3(0.004669, -0.445692, -0.165899),
                        vec3(0.607166, -0.571184, 0.377880),
                        vec3(-0.607685, -0.352123, -0.663045),
                        vec3(-0.235328, -0.142338, 0.925718),
                        vec3(-0.023743, -0.297281, -0.392438),
                        vec3(0.918790, 0.056215, 0.092624),
                        vec3(0.608966, -0.385235, -0.108280),
                        vec3(-0.802881, 0.225105, 0.361339),
                        vec3(-0.070376, 0.303049, -0.905118),
                        vec3(-0.503922, -0.475265, 0.177892),
                        vec3(0.035096, -0.367809, -0.475295),
                        vec3(-0.316874, -0.374981, -0.345988),
                        vec3(-0.567278, -0.297800, -0.271889),
                        vec3(-0.123325, 0.197851, 0.626759),
                        vec3(0.852626, -0.061007, -0.144475) };

#define TAP_SIZE 0.02
#define NUM_TAPS 16
#define THRESHOLD 0.1
#define SCALE 1.0

/*
float filter(float x)
{
    return max(0, 1.0 - x*x);
}
*/

void main()
{
    vec3 viewPos = texture(positionMap, out_Tex).xyz;
    vec3 viewNorm = texture(normalMap, out_Tex).xyz;
    float total = 0.0;
    for (uint i = 0; i < NUM_TAPS; i++) {
        vec3 offset = TAP_SIZE * taps[i];
        vec2 offTex = out_Tex + offset.st;
        vec3 offPos = texture(positionMap, offTex.st).xyz; // view space position of offset point

        vec3 diff = offPos - viewPos;
        float distance = length(diff);
        vec3 diffnorm = normalize(diff);

        float occlusion = max(0.0, dot(viewNorm, diffnorm)) * SCALE / (1.0 + distance);
        total += 1.0 - occlusion;
    }
    total /= NUM_TAPS;
    out_Color = vec4(total, total, total, 1.0);
}

#endif
