#version 430 core

#ifdef _COMPUTE_

// compute shader which populates the light grid and light index lists, then does the lighting computation

// tiles are 32x32 pixels
#define TILE_SIZE 32
layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;

uniform layout(binding = 0) sampler2DMS ambient_map;
uniform layout(binding = 1) sampler2DMS diffuse_map;
uniform layout(binding = 2) sampler2DMS specular_map;
uniform layout(binding = 3) sampler2DMS normal_map;
uniform layout(binding = 4) sampler2DMS depth_map;

uniform layout(binding = 5) samplerBuffer lights_buffer;
uniform layout(location = 5) uint num_lights;
// light struct is 7 bytes
#define LIGHT_SIZE 7

uniform layout(location = 3) mat4 camMvMatrix;
uniform layout(location = 4) mat4 camPjMatrix;

shared int numLightsForTile;
shared int lightsForTile[1024];

uniform layout(binding = 0, rgba32f) coherent restrict writeonly image2D out_Image;
uniform layout(binding = 1, r32f) coherent restrict writeonly image2D out_Depth;

void main()
{
    const ivec2 tile_id = ivec2(gl_WorkGroupID);
    const ivec2 thread_id = ivec2(gl_LocalInvocationID);
    const ivec2 i_Tex = ivec2(gl_GlobalInvocationID); // coordinates for indexing into output images
    numLightsForTile = 0; // does this need to be protected from shared writes?

    // first, we need to populate the list of lights that affect the current tile
    // use the thread index to iterate through the light list
    const int lightIdx = thread_id.y * TILE_SIZE + thread_id.x;
    if (lightIdx < num_lights) {
        // we need to consider this light for the tile
        float lightx = texelFetch(lights_buffer, lightIdx*LIGHT_SIZE).r;
        float lighty = texelFetch(lights_buffer, lightIdx*LIGHT_SIZE+1).r;
        float lightz = texelFetch(lights_buffer, lightIdx*LIGHT_SIZE+2).r;
        float lightSize = texelFetch(lights_buffer, lightIdx*LIGHT_SIZE+6).r;

        // do some sort of sphere/AABB screen space bounds calculation
        // for now, assume true
        int index = atomicAdd(numLightsForTile, 1); // atomic increment the offset counter
        lightsForTile[index] = lightIdx;
    }
    barrier(); // ensures that all threads in the work group have processed their light
    // we should have lightForTile populated with the index of each light which affects the tile

    // now do the lighting calculation for the fragment
    // currently not SSAA, need to fix (more threads?)
    vec4 ambient_Color = texelFetch(ambient_map, i_Tex, 0);
    vec4 diffuse_Color = texelFetch(diffuse_map, i_Tex, 0);
    vec4 specular_Color = texelFetch(specular_map, i_Tex, 0);
    vec3 view_Normal = texelFetch(normal_map, i_Tex, 0).xyz;

    float fragDepth = texelFetch(depth_map, i_Tex, 0).r;
    vec3 start_Pos = vec3(vec2(i_Tex)/textureSize(depth_map), fragDepth);
    vec3 ndc_Pos = (2.0 * start_Pos) - 1.0;
    vec4 unproject = inverse(camPjMatrix) * vec4(ndc_Pos, 1.0);
    vec3 viewPos = unproject.xyz / unproject.w;

    vec3 diffuseColor = vec3(0);
    vec3 specularColor = vec3(0);

    // for each light
    for (int i = 0; i < numLightsForTile; i++) {
        // get parameters from buffer
        float lightx = texelFetch(lights_buffer, lightsForTile[i]*LIGHT_SIZE).r;
        float lighty = texelFetch(lights_buffer, lightsForTile[i]*LIGHT_SIZE+1).r;
        float lightz = texelFetch(lights_buffer, lightsForTile[i]*LIGHT_SIZE+2).r;
        float lightr = texelFetch(lights_buffer, lightsForTile[i]*LIGHT_SIZE+3).r;
        float lightg = texelFetch(lights_buffer, lightsForTile[i]*LIGHT_SIZE+4).r;
        float lightb = texelFetch(lights_buffer, lightsForTile[i]*LIGHT_SIZE+5).r;
        float lightSize = texelFetch(lights_buffer, lightsForTile[i]*LIGHT_SIZE+6).r;

        vec3 lightPower = vec3(lightr, lightg, lightb);
        // calculate view space position
        vec4 lightPos = camMvMatrix * vec4(lightx, lighty, lightz, 1.0);

        // do the lighting calculation
        vec3 light_Direction = lightPos.xyz - viewPos;
        float lightDistance = length(light_Direction);

        //        if (lightDistance > lightSize) {
        //            continue;
        //        }

        light_Direction = normalize(light_Direction);
        float distance2 = lightDistance * lightDistance;
        float ndotl = dot(view_Normal, light_Direction); // for some reason this dot product is causing gl state mismatch fragment shader recompile warning
        float intensity = clamp(ndotl, 0, 1);

        //        if (light_Direction.y < 0) {
        //            out_Color = vec3(1.0, 0, 0);
        //        } else {
        //            out_Color = vec3(0, 1.0, 0);
        //        }

        diffuseColor += intensity * lightPower / distance2;

        vec3 halfVector = normalize(light_Direction + vec3(0, 0, -1));
        float ndoth = dot(view_Normal, halfVector);
        intensity = pow( clamp(ndoth, 0, 1), specular_Color.a);

        specularColor += intensity * lightPower / distance2;
    }
    vec3 out_Color = diffuseColor * diffuse_Color.rgb + specularColor * specular_Color.rgb;
    imageStore(out_Image, i_Tex, vec4(out_Color, 1.0));

    //out_Color = vec4(texture(depth_map, out_Tex).rgb, 1.0);
    imageStore(out_Depth, i_Tex, vec4(fragDepth, 0, 0, 0));
}

#endif //_COMPUTE_
