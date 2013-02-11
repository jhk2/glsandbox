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

void main()
{
	out_Tex = in_Tex.xy;
	
	// position of object in camera space
	out_Pos = mvMatrix * vec4(in_Pos, 1);
	
	gl_Position = pjMatrix * out_Pos;
	
	// position in light space
	light_Tex = lightpj * lightmv * vec4(in_Pos, 1);
	
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

const float depthBias = 0.005;

in vec4 out_Pos;
in vec2 out_Tex;
in vec3 out_Norm;
in vec4 light_Tex;
in vec4 lightPos;
out vec4 out_Color;


uniform float Ka;
uniform float Kd;

uniform sampler2D map_Ka;
uniform sampler2D map_Kd;
uniform sampler2D shadow;

void main() {
	float kshadow = 1.0f;
	vec3 lTex = (light_Tex.xyz / light_Tex.w) * 0.5 + vec3(0.5);
	bvec2 outside = greaterThan(lTex.xy,vec2(1.0,1.0));
	bvec2 inside = lessThan(lTex.xy,vec2(0,0));
	if ((texture(shadow, lTex.xy).r < lTex.z - depthBias)||any(outside)||any(inside)) {
		kshadow = 0.0f;
	}
	
	// vector from object to light
	vec4 objtolight = lightPos - out_Pos;
	objtolight = normalize(objtolight);
	
	// diffuse lighting is n dot l
	vec4 diffuseTex = texture(map_Kd, out_Tex);
	vec3 diffuse = dot(objtolight.xyz, out_Norm) * Kd * kshadow * diffuseTex.xyz;
	//vec3 diffuse = Kd * diffuseTex.xyz;
	vec4 ambientTex = texture(map_Ka, out_Tex);
	vec3 ambient = Ka * ambientTex.xyz;
	
	out_Color = vec4(diffuse + ambient, min(1.0, diffuseTex.w + ambientTex.w));
	//out_Color = vec4(kshadow*ambientTex.xyz, 1.0f);
	
	//out_Color = texture(shadow, lTex.xy);
	//out_Color = texture(shadow, (light_Tex.xy / light_Tex.w) * 0.5 + vec2(0.5));
	//out_Color = texture(shadow, light_Tex.xy * 0.5 + vec2(0.5));
	//out_Color = vec4((light_Pos.xy / light_Pos.w) * 0.5 + vec2(0.5), 0, 1.0);
	//out_Color = vec4(light_Tex * 0.5 + vec2(0.5), 0, 1.0);
}
#endif
