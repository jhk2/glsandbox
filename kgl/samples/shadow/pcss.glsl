// PCSS whitepaper at http://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf
// The PCSS code itself is just an OpenGL adapted version of the D3D code in this paper

#version 430 core

uniform mat4 lightmv;
#define M_PI 3.1415926535897932384626433832795

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

const float depthBias = 0.5;

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
uniform sampler2DShadow shadow;
uniform sampler2D shadowTex;

//uniform float lightNearPlane;
//uniform float lightFarPlane;


#define BLOCKER_SEARCH_NUM_SAMPLES 16
#define PCF_NUM_SAMPLES 64
#define LIGHT_SIZE 0.1

// poisson disk for sampling
const vec2 poissonDisk[64] = {
	vec2( -0.04117257f, -0.1597612f ),
	vec2( 0.06731031f, -0.4353096f ),
	vec2( -0.206701f, -0.4089882f ),
	vec2( 0.1857469f, -0.2327659f ),
	vec2( -0.2757695f, -0.159873f ),
	vec2( -0.2301117f, 0.1232693f ),
	vec2( 0.05028719f, 0.1034883f ),
	vec2( 0.236303f, 0.03379251f ),
	vec2( 0.1467563f, 0.364028f ),
	vec2( 0.516759f, 0.2052845f ),
	vec2( 0.2962668f, 0.2430771f ),
	vec2( 0.3650614f, -0.1689287f ),
	vec2( 0.5764466f, -0.07092822f ),
	vec2( -0.5563748f, -0.4662297f ),
	vec2( -0.3765517f, -0.5552908f ),
	vec2( -0.4642121f, -0.157941f ),
	vec2( -0.2322291f, -0.7013807f ),
	vec2( -0.05415121f, -0.6379291f ),
	vec2( -0.7140947f, -0.6341782f ),
	vec2( -0.4819134f, -0.7250231f ),
	vec2( -0.7627537f, -0.3445934f ),
	vec2( -0.7032605f, -0.13733f ),
	vec2( 0.8593938f, 0.3171682f ),
	vec2( 0.5223953f, 0.5575764f ),
	vec2( 0.7710021f, 0.1543127f ),
	vec2( 0.6919019f, 0.4536686f ),
	vec2( 0.3192437f, 0.4512939f ),
	vec2( 0.1861187f, 0.595188f ),
	vec2( 0.6516209f, -0.3997115f ),
	vec2( 0.8065675f, -0.1330092f ),
	vec2( 0.3163648f, 0.7357415f ),
	vec2( 0.5485036f, 0.8288581f ),
	vec2( -0.2023022f, -0.9551743f ),
	vec2( 0.165668f, -0.6428169f ),
	vec2( 0.2866438f, -0.5012833f ),
	vec2( -0.5582264f, 0.2904861f ),
	vec2( -0.2522391f, 0.401359f ),
	vec2( -0.428396f, 0.1072979f ),
	vec2( -0.06261792f, 0.3012581f ),
	vec2( 0.08908027f, -0.8632499f ),
	vec2( 0.9636437f, 0.05915006f ),
	vec2( 0.8639213f, -0.309005f ),
	vec2( -0.03422072f, 0.6843638f ),
	vec2( -0.3734946f, -0.8823979f ),
	vec2( -0.3939881f, 0.6955767f ),
	vec2( -0.4499089f, 0.4563405f ),
	vec2( 0.07500362f, 0.9114207f ),
	vec2( -0.9658601f, -0.1423837f ),
	vec2( -0.7199838f, 0.4981934f ),
	vec2( -0.8982374f, 0.2422346f ),
	vec2( -0.8048639f, 0.01885651f ),
	vec2( -0.8975322f, 0.4377489f ),
	vec2( -0.7135055f, 0.1895568f ),
	vec2( 0.4507209f, -0.3764598f ),
	vec2( -0.395958f, -0.3309633f ),
	vec2( -0.6084799f, 0.02532744f ),
	vec2( -0.2037191f, 0.5817568f ),
	vec2( 0.4493394f, -0.6441184f ),
	vec2( 0.3147424f, -0.7852007f ),
	vec2( -0.5738106f, 0.6372389f ),
	vec2( 0.5161195f, -0.8321754f ),
	vec2( 0.6553722f, -0.6201068f ),
	vec2( -0.2554315f, 0.8326268f ),
	vec2( -0.5080366f, 0.8539945f )
};

float gaussian(float radius, float x) {
	return 1.0f;
	// radius should be about 3*sigma
	float sigma = radius / 3;
	return (1/(sigma*sqrt(2*M_PI))) * exp(-0.5*x*x/(sigma*sigma));
}

// pseudorandom number generator
float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(vec4 co){
	float dot_product = dot(co, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

// parallel plane estimation
float penumbraSize(float zReceiver, float zBlocker) {
	return (zReceiver - zBlocker) / zBlocker;
}

vec2 findBlocker(vec2 texCoord, float zReceiver) {
	float searchWidth = LIGHT_SIZE;
	float blockerSum = 0;
	float numBlockers = 0;
	
	for (int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; i++) {
		vec2 coord = texCoord + poissonDisk[i] * searchWidth;
		float smap = texture(shadowTex, coord).r;
		if (smap < zReceiver) {
			blockerSum += smap;
			numBlockers++;
		}
	}
	return vec2(blockerSum / numBlockers, numBlockers);
}

float pcf(vec2 texCoord, float zReceiver, float filterRadius) {
	float sum = 0;
	float theta = rand(vec4(texCoord, gl_FragCoord.xy));
	mat2 rotation = mat2(vec2(cos(theta), sin(theta)), vec2(-sin(theta), cos(theta)));
	for (int i = 0; i < PCF_NUM_SAMPLES; i++) {
		vec2 offset = (rotation*poissonDisk[i]) * filterRadius;
		vec2 texOffset = texCoord + offset;
		bvec2 outside = greaterThan(texOffset, vec2(1.0,1.0));
		bvec2 inside = lessThan(texOffset, vec2(0,0));
		float ishadow = texture(shadow, vec3(texOffset, zReceiver));
		//float gauss = gaussian(filterRadius, length(offset));
		sum += ((any(outside)||any(inside)) ? 1.0f : ishadow);
	}
	return sum / PCF_NUM_SAMPLES;
}

float pcss(vec3 texCoord) {
	// blocker search
	vec2 blockers = findBlocker(texCoord.xy, texCoord.z); // x has the average depth, y has the total number of blockers
	if (blockers.y < 1) {
		// no blockers so no shadowing
		return 1.0f;
	}
	
	float penumbraRatio = penumbraSize(texCoord.z, blockers.x);
	float filterRadius = penumbraRatio * LIGHT_SIZE;
	
	return pcf(texCoord.xy, texCoord.z, filterRadius);
}

void main() {
	vec3 lTex = light_Tex.xyz / light_Tex.w;
	bvec2 outside = greaterThan(lTex.xy,vec2(1.0,1.0));
	bvec2 inside = lessThan(lTex.xy,vec2(0,0));
	// for sampler2DShadow
	float kshadow = pcss(lTex);
	//float kshadow = pcf(lTex.xy, lTex.z, 0.05);
	
	//float smap = texture(shadowTex, lTex.xy).r;
	
	// vector from object to light
	vec4 objtolight = lightPos - out_Pos;
	objtolight = normalize(objtolight);
	
	// diffuse lighting is n dot l
	vec4 diffuseTex = texture(map_Kd, out_Tex);
	float ndotl = max(dot(objtolight.xyz, out_Norm), 0);
	vec3 diffuse = ndotl * Kd * kshadow * diffuseTex.xyz;
	
	vec4 ambientTex = texture(map_Ka, out_Tex);
	vec3 ambient = Ka * ambientTex.xyz;
	
	out_Color = vec4(diffuse + ambient, min(1.0, diffuseTex.w + ambientTex.w));
}
#endif