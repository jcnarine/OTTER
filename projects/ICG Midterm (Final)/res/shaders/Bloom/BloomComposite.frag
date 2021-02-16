#version 420
/*

Referenced Code from SpriteLib and https://learnopengl.com/Advanced-Lighting/Bloom

Justification: Used the code as a base to understand how to composite the bloom effect and the scene 

Modifications: I modified the code to include the referenced code from learnopengl with gamma correction
			   I also included the result, instead of the frag_color 

*/
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
	
	result = 1.0 - (1.0 - colorA.rgb) * (1.0 - colorB.rgb);

	result = pow(result, vec3(1.0/gamma));

	frag_color = vec4(result, 1.0);
}