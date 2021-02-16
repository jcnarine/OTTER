#version 420

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