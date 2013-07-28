#version 430 core
#define M_PI 3.1415926535897932384626433832795

uniform layout(location = 2) mat4 eyePj; // perspective camera used to render original scene

#ifdef _VERTEX_
uniform layout(location = 0) mat4 mvMatrix;
uniform layout(location = 1) mat4 pjMatrix;

in layout(location = 0) vec3 in_Pos;
in layout(location = 1) vec3 in_Tex;

out vec2 clip_XY;
out vec2 out_Tex;

void main()
{
    out_Tex = in_Tex.st;
    vec4 clip_Pos = pjMatrix * mvMatrix * vec4(in_Pos, 1.0);
    clip_XY = clip_Pos.xy;
    gl_Position = clip_Pos;
}

#endif

#ifdef _FRAGMENT_

uniform layout(binding = 0) sampler2D positionMap;
uniform layout(binding = 1) sampler2D normalMap;
uniform layout(binding = 2) sampler2D depthMap;

in vec4 clip_XY;
in vec2 out_Tex;

out vec4 out_Color;

// sampling radius is in view space
#define SAMPLING_RADIUS 0.5
#define NUM_SAMPLING_DIRECTIONS 8
// sampling step is in texture space
#define SAMPLING_STEP 0.004
#define NUM_SAMPLING_STEPS 4
#define THRESHOLD 0.1
#define SCALE 1.0
#define TANGENT_BIAS 0.2

float filter(float x)
{
    return max(0, 1.0 - x*x);
}

// HBAO paper http://rdimitrov.twistedsanity.net/HBAO_SIGGRAPH08.pdf
// HBAO SIGGRAPH presentation http://developer.download.nvidia.com/presentations/2008/SIGGRAPH/HBAO_SIG08b.pdf

void main()
{
    float start_Z = texture(depthMap, out_Tex).r; // returns value (z/w+1)/2
    vec3 start_Pos = vec3(out_Tex, start_Z);
    vec3 ndc_Pos = (2.0 * start_Pos) - 1.0; // transform to normalized device coordinates xyz/w
    // reconstruct view space position
    vec4 unproject = inverse(eyePj) * vec4(ndc_Pos, 1.0);
    vec3 viewPos = unproject.xyz / unproject.w; // 3d view space position P
    vec3 viewNorm = texture(normalMap, out_Tex).xyz; // 3d view space normal N
    float total = 0.0;
    float sample_direction_increment = 2 * M_PI / NUM_SAMPLING_DIRECTIONS;
    for (uint i = 0; i < NUM_SAMPLING_DIRECTIONS; i++) {
        // no jittering or randomization of sampling direction just yet
        float sampling_angle = i * sample_direction_increment; // azimuth angle theta in the paper
        vec2 sampleDir = vec2(cos(sampling_angle), sin(sampling_angle));
        // we will now march along sampleDir and calculate the horizon
        // horizon starts with the tangent plane to the surface, whose angle we can get from the normal
        float tangentAngle = acos(dot(vec3(sampleDir, 0), viewNorm)) - (0.5 * M_PI) + TANGENT_BIAS;
        float horizonAngle = tangentAngle;
        vec3 lastDiff = vec3(0);
        for (uint j = 0; j < NUM_SAMPLING_STEPS; j++) {
            // march along the sampling direction and see what the horizon is
            vec2 sampleOffset = float(j+1) * SAMPLING_STEP * sampleDir;
            vec2 offTex = out_Tex + sampleOffset;

            float off_start_Z = texture(depthMap, offTex.st).r;
            vec3 off_start_Pos = vec3(offTex, off_start_Z);
            vec3 off_ndc_Pos = (2.0 * off_start_Pos) - 1.0;
            vec4 off_unproject = inverse(eyePj) * vec4(off_ndc_Pos, 1.0);
            vec3 off_viewPos = off_unproject.xyz / off_unproject.w;
            // we now have the view space position of the offset point
            vec3 diff = off_viewPos.xyz - viewPos.xyz;
            if (length(diff) < SAMPLING_RADIUS) {
                // skip samples which are outside of our local sampling radius
                lastDiff = diff;
                float elevationAngle = atan(diff.z / length(diff.xy));
                horizonAngle = max(horizonAngle, elevationAngle);
            }
        }
        // the paper uses this attenuation but I like the other way better
        //float normDiff = length(lastDiff) / SAMPLING_RADIUS;
        //float attenuation = 1 - normDiff*normDiff;
        float attenuation = 1.0 / (1 + length(lastDiff));
        // now compare horizon angle to tangent angle to get ambient occlusion
        float occlusion = clamp(attenuation * (sin(horizonAngle) - sin(tangentAngle)), 0.0, 1.0);
        total += 1.0 - occlusion;
    }
    total /= NUM_SAMPLING_DIRECTIONS;

    out_Color = vec4(total, total, total, 1.0);
}

#endif
