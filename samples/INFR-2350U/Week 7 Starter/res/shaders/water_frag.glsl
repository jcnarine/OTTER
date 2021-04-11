#version 420

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inFragPosLightSpace;

in vec4 gl_FragCoord;

struct DirectionalLight
{
	//Light direction (defaults to down, to the left, and a little forward)
	vec4 _lightDirection;

	//Generic Light controls
	vec4 _lightCol;

	//Ambience controls
	vec4 _ambientCol;
	float _ambientPow;
	
	//Power controls
	float _lightAmbientPow;
	float _lightSpecularPow;
	
	float _shadowBias;
};

layout (std140, binding = 0) uniform u_Lights
{
	DirectionalLight sun;
};

uniform sampler2D s_Diffuse;
uniform sampler2D s_Diffuse2;
uniform sampler2D s_Specular;
uniform sampler2D s_NormalMap;

uniform float u_TextureMix;
uniform vec3  u_CamPos;

layout(binding = 29) uniform sampler2D s_DepthBuffer;

uniform float u_waterTransparency;


uniform float u_WindowWidth;
uniform float u_WindowHeight;

uniform vec4 u_DeepColor;
uniform vec4 u_MidColor;
uniform vec4 u_ShoreColor;

uniform float u_shoreCutoff;
uniform float u_midCutoff;

out vec4 frag_color;

float linearize_depth(float d, float zNear, float zFar){
	float z_n = 2.0 * d - 1.0;
	return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

//https://learnopengl.com/Advanced-Lighting/Advanced-Lighting

void main() {

	vec2 screenUV = vec2(gl_FragCoord.x / u_WindowWidth, gl_FragCoord.y / u_WindowHeight );

	float expectedDepth = linearize_depth(texture(s_DepthBuffer, screenUV).x, 0.01, 1000.0);

	float actualDepth = linearize_depth(gl_FragCoord.z, 0.01, 1000.0);

	float depthDiff = expectedDepth - actualDepth;

	float shoreColorMod = 0.0;
	float midColorMod = 0.0;
	float deepColorMod = 0.0;

	if (depthDiff <= u_shoreCutoff)
	{
		shoreColorMod = 1.0;

	}else if (depthDiff<= u_midCutoff){

		midColorMod = 1.0;

	}else{
		deepColorMod = 1.0;
	}

	// Normals 
	vec3 inNormal = (normalize(texture(s_NormalMap, inUV).rgb) * 2.0) - 1.0;

	// Diffuse
	vec3 N = normalize(inNormal);
	vec3 lightDir = normalize(-sun._lightDirection.xyz);
	float dif = max(dot(N, lightDir), 0.0);
	vec3 diffuse = dif * sun._lightCol.xyz;// add diffuse intensity

	// Specular
	vec3 viewDir  = normalize(u_CamPos - inPos);
	vec3 h        = normalize(lightDir + viewDir);

	// Get the specular power from the specular map
	float texSpec = texture(s_Specular, inUV).x;
	float spec = pow(max(dot(N, h), 0.0), 4.0); // Shininess coefficient (can be a uniform)
	vec3 specular = sun._lightSpecularPow * texSpec * spec * sun._lightCol.xyz; // Can also use a specular color

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor1 = texture(s_Diffuse, inUV);
	vec4 textureColor2 = texture(s_Diffuse2, inUV);
	vec4 textureColor = mix(textureColor1, textureColor2, u_TextureMix);

	vec3 result = (
		(sun._ambientPow * sun._ambientCol.xyz) + // global ambient light
		(diffuse + specular) // light factors from our single light
		) * inColor * mix(textureColor, u_ShoreColor, shoreColorMod).rgb // Object color
		*(shoreColorMod 
		+ (u_MidColor.rgb * midColorMod)
		+ (u_DeepColor.rgb * deepColorMod));

	frag_color = vec4(result, u_waterTransparency
			* ((u_ShoreColor.a, shoreColorMod)// Object color
			+ (u_MidColor.a * midColorMod)
			+ (u_DeepColor.a * deepColorMod)));
	
}