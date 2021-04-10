<<<<<<< HEAD
#version 420

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

uniform sampler2D s_Diffuse;
uniform sampler2D s_Diffuse2;
uniform sampler2D s_Specular;

uniform float u_TextureMix;

layout(location = 0) out vec4 outColors;
layout(location = 1) out vec3 outNormals;
layout(location = 2) out vec3 outSpecs;
layout(location = 3) out vec3 outPositions;

void main(){

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor1 = texture(s_Diffuse, inUV);
	vec4 textureColor2 = texture(s_Diffuse2, inUV);
	vec4 textureColor = mix(textureColor1, textureColor2, u_TextureMix);

	//output texture color
	outColors = textureColor;

	//converts normal color from [-1,1] to [0,1]
	outNormals = (normalize(inNormal)*0.5)+0.5;

	//outputs specular colors
	outSpecs = texture(s_Specular, inUV).rgb;

	//outputs the viewspace position
	outPositions = inPos;
=======
#version 420

//Data for this model
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

//The albedo textures for the model being drawn
uniform sampler2D s_Diffuse;
uniform sampler2D s_Diffuse2;
uniform sampler2D s_Specular;
uniform float u_TextureMix;

//MRT
//Multi Render Target
//We can render color to ALL of these targets
layout(location = 0) out vec4 outColors;
layout(location = 1) out vec3 outNormals;
layout(location = 2) out vec3 outSpecs;
layout(location = 3) out vec3 outPositions;

void main() 
{
	// Get the albedo from the diffuse / albedo map
	vec4 textureColor1 = texture(s_Diffuse, inUV);
	vec4 textureColor2 = texture(s_Diffuse2, inUV);
	vec4 textureColor = mix(textureColor1, textureColor2, u_TextureMix);

	//Outputs the texture colors
	outColors = textureColor;

	//Outputs the normal colors
	//[-1, 1] -> InNormals range
	//[0, 1] -> OutNormals Range (we want them here)
	//So we convert from [-1,1] to [0,1]
	outNormals = (normalize(inNormal) * 0.5) + 0.5;

	//Outputs the specular colors
	outSpecs = texture(s_Specular, inUV).rgb;

	//Outputs the viewspace positions
	outPositions = inPos;
>>>>>>> master
}