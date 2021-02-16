#version 420

layout(binding = 0) uniform sampler2D uScene;
layout(binding = 1) uniform sampler2D uBloom;

in vec2 TexCoords;

out vec4 frag_color;

void main() 
{
	const float gamma = 2.2;

	vec4 colorA = texture(uScene, TexCoords);
	vec4 colorB = texture(uBloom, TexCoords);

	vec3 result = colorA.rgb + colorB.rgb;

	//frag_color = 1.0 - (1.0 - colorA) * (1.0 - colorB);
	
	result = 1.0 - (1.0 - colorA.rgb) * (1.0 - colorB.rgb);

	result = pow(result, vec3(1.0/gamma));

	frag_color = vec4(result, 1.0);
}