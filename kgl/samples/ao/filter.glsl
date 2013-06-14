#version 430 core

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
uniform layout(binding = 0) sampler2D tex;

in vec2 out_Tex;

out vec4 out_Color;


const ivec2 offset [9] = { ivec2(-1,1),   ivec2(0,1),     ivec2(1,1),
                     ivec2(-1,0),   ivec2(0,0),     ivec2(1,0),
                     ivec2(-1,-1),  ivec2(0,-1),    ivec2(1,-1) };

// box filter
float filter(vec2 pos)
{
    float total = 0.0f;
    for (uint i = 0; i < 9; i++) {
        float val = textureOffset(tex, pos, offset[i]).r;
        total += val;
    }
    total /= 9.0f;
    return total;
}

void main()
{
    out_Color = vec4(filter(out_Tex), 0, 0, 1.0);
}

#endif
