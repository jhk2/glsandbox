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
uniform layout(binding = 4) sampler2D depth_map;

uniform layout(binding = 5) samplerBuffer lights_buffer;
uniform layout(location = 5) uint num_lights;
// light struct is 7 bytes
#define LIGHT_SIZE 7

uniform layout(location = 3) mat4 camMvMatrix;
uniform layout(location = 4) mat4 camPjMatrix;

in vec2 out_Tex;

out layout(location = 0) vec3 out_Color;

void main() {
    vec4 ambient_Color = texture(ambient_map, out_Tex);
    vec4 diffuse_Color = texture(diffuse_map, out_Tex);
    vec4 specular_Color = texture(specular_map, out_Tex);
    vec3 view_Normal = texture(normal_map, out_Tex).xyz;

    float fragDepth = texture(depth_map, out_Tex).r;
    vec3 start_Pos = vec3(out_Tex, fragDepth);
    vec3 ndc_Pos = (2.0 * start_Pos) - 1.0;
    vec4 unproject = inverse(camPjMatrix) * vec4(ndc_Pos, 1.0);
    vec3 viewPos = unproject.xyz / unproject.w;

    vec3 diffuseColor = vec3(0);
    vec3 specularColor = vec3(0);

    // for each light
    for (int i = 0; i < num_lights; i++) {
        // get parameters from buffer
        float lightx = texelFetch(lights_buffer, i*LIGHT_SIZE).r;
        float lighty = texelFetch(lights_buffer, i*LIGHT_SIZE+1).r;
        float lightz = texelFetch(lights_buffer, i*LIGHT_SIZE+2).r;
        float lightr = texelFetch(lights_buffer, i*LIGHT_SIZE+3).r;
        float lightg = texelFetch(lights_buffer, i*LIGHT_SIZE+4).r;
        float lightb = texelFetch(lights_buffer, i*LIGHT_SIZE+5).r;
        float lightSize = texelFetch(lights_buffer, i*LIGHT_SIZE+6).r;

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
        float ndotl = dot(view_Normal, light_Direction);
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
    out_Color = diffuseColor * diffuse_Color.rgb + specularColor * specular_Color.rgb;

    //out_Color = view_Normal;
    //out_Color = vec4(texture(depth_map, out_Tex).rgb, 1.0);
    gl_FragDepth = texture(depth_map, out_Tex).r;
}

#endif // _FRAGMENT_
