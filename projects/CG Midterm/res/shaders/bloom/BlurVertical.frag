#version 420

layout(binding = 0) uniform sampler2D uTex; //Source image
uniform float u_BlurValue; //1.0 / Window_Height

out vec4 frag_color;

in vec2 TexCoords;

void main() 
{
	//Sample pixels in a horizontal row
	//Weight should add up to 1
	vec4 result = vec4(0.0, 0.0, 0.0, 0.0);

	result += texture(uTex, vec2(TexCoords.x, TexCoords.y - 4.0 * u_BlurValue)) * 0.06;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y - 3.0 * u_BlurValue)) * 0.09;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y - 2.0 * u_BlurValue)) * 0.12;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y - 		 u_BlurValue)) * 0.15;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y		            )) * 0.16;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y +		 u_BlurValue)) * 0.15;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y + 2.0 * u_BlurValue)) * 0.12;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y + 3.0 * u_BlurValue)) * 0.09;
	result += texture(uTex, vec2(TexCoords.x, TexCoords.y + 4.0 * u_BlurValue)) * 0.06;

	frag_color= result;

}