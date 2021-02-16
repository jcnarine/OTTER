#version 420
/*

Referenced Code from SpriteLib 

Justification: Used the code as a base to understand the process of blurring and how to properly blur the scene 

Modifications: I changed it from frag_color to result, as I only wanted one assignment for frag_color
			   I also added a new array of weights and a for loop to replace the bulk of code 
			   (This idea was from looking at https://learnopengl.com/Advanced-Lighting/Bloom)
			   This allow for more automation and to make the code cleaner
*/

layout(binding = 0) uniform sampler2D uTex; //Source image
uniform float u_BlurValue; //1.0 / Window_Width
out vec4 frag_color;

in vec2 TexCoords;
uniform float Weight[5] = float[] (0.0, 0.15, 0.12, 0.09, 0.06);

void main() 
{
	//Sample pixels in a horizontal row
	//Weight should add up to 1
	vec4 result = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i=4; i>0; --i){
	result+= texture(uTex, vec2(TexCoords.x - i * u_BlurValue, TexCoords.y)) * Weight[i];
	}
	result+= texture(uTex, vec2(TexCoords.x,				   TexCoords.y)) * 0.16;
	for (int i=1; i<=4; ++i){
	result+= texture(uTex, vec2(TexCoords.x + i * u_BlurValue, TexCoords.y)) * Weight[i];
	}

	frag_color= result;

}



	