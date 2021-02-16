#version 420
/*

Referenced Code from SpriteLib 

Justification: Used the code as a base to understand how to apply multiple framebuffers and the base bloom code

Modifications: I modified all the code for pushing the shaders into the Shader::Sptr Vector,
			   I also changed all the variables names to show the change from the Spritelib to the Otter framework,
			   using the PostEffect class created through the tutorial
			   Also added glm:: and new function calls when neccessary
*/
layout(binding = 0) uniform sampler2D uTex; //Source image
uniform float u_Threshold;

out vec4 frag_color;

in vec2 TexCoords;

void main() 
{
	vec4 color = texture(uTex, TexCoords);
	
	float luminance = (color.r + color.g + color.b) / 3.0;
	
	if (luminance > u_Threshold) 
	{
		frag_color = color;
	}
	else
	{
		frag_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}