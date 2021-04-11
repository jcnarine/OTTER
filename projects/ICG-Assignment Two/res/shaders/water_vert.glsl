#version 410

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;

uniform float u_Time;
uniform float u_SineFactor;
uniform float u_TimeFactor;
uniform float u_HeightFactor;


uniform mat4 u_ModelViewProjection;
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;
uniform vec3 u_LightPos;



void main() {

	// Lecture 5
	// Pass vertex pos in world space to frag shader
	outPos = (u_Model * vec4(inPosition, 1.0)).xyz;

	////LECTURE 10b
	vec3 vert = inPosition;
	
	//vert.y = texture(myTextureSampler, vertex_uv).r;
	vert.z = (sin(vert.x * u_HeightFactor + u_Time * u_TimeFactor)) * u_SineFactor;
	gl_Position = u_ModelViewProjection * vec4(vert, 1.0);

	// Normals
	outNormal = u_NormalMatrix * inNormal;

	// Pass our UV coords to the fragment shader
	outUV = inUV;

	///////////
	outColor = inColor;

}

