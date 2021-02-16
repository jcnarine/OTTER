#version 410

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

uniform sampler2D s_Diffuse;
uniform sampler2D s_Diffuse2;
uniform sampler2D s_Specular;

uniform vec3  u_AmbientCol;
uniform float u_AmbientStrength;

uniform vec3  u_LightPos;
uniform vec3  u_LightCol;
uniform float u_AmbientLightStrength;
uniform float u_SpecularLightStrength;
uniform float u_Shininess;
uniform float u_LightAttenuationConstant;
uniform float u_LightAttenuationLinear;
uniform float u_LightAttenuationQuadratic;
uniform float u_TextureMix;

uniform float lightIntensity = 40.0;

//LECTURE 10c - Toon shading
uniform int bands = 6;
uniform float scaleValue = 2;
uniform bool u_ToonShading=false;

uniform vec3  u_CamPos;

uniform int u_LightingToggle;
uniform bool u_TextureColor;
uniform float u_BrightnessThreshold;
uniform float u_BlurValues;

out vec4 frag_color;

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	// Lecture 5
	vec3 ambient = u_AmbientLightStrength * u_LightCol;

	// Diffuse
	vec3 N = normalize(inNormal);
	vec3 lightDir = normalize(u_LightPos - inPos);

	float dif = max(dot(N, lightDir), 0.0);
	vec3 diffuse = dif * u_LightCol;// add diffuse intensity

	//Attenuation
	float dist = length(u_LightPos - inPos);
	float attenuation = 1.0f / (
		u_LightAttenuationConstant + 
		u_LightAttenuationLinear * dist +
		u_LightAttenuationQuadratic * dist * dist);

	// Specular
	vec3 viewDir  = normalize(u_CamPos - inPos);
	vec3 h        = normalize(lightDir + viewDir);

	// Get the specular power from the specular map
	float texSpec = texture(s_Specular, inUV).x;
	float spec = pow(max(dot(N, h), 0.0), u_Shininess); // Shininess coefficient (can be a uniform)
	vec3 specular = u_SpecularLightStrength * texSpec * spec * u_LightCol; // Can also use a specular color

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor1 = texture(s_Diffuse, inUV);
	vec4 textureColor2 = texture(s_Diffuse2, inUV);
	vec4 textureColor = mix(textureColor1, textureColor2, u_TextureMix);
	
	vec3 result;

	switch (u_LightingToggle){
		case 1:
			result = inColor;
			break;
		case 2:
			result = ((u_AmbientCol * u_AmbientStrength) + (ambient + diffuse) * attenuation) * inColor;
			break;
		case 3:
			result = ((specular + diffuse) * attenuation) * inColor;
			break;
		case 4:
			result = (
				(u_AmbientCol * u_AmbientStrength) + // global ambient light
				(ambient + diffuse + specular) * attenuation // light factors from our single light
				) * inColor; // Object color
			break;
		case 5:
			if(u_ToonShading){

				vec3 L = normalize(u_LightPos - inPos);
				vec3 V = normalize(u_CamPos - inPos);

				float diffuse = max(0, dot(L, inNormal));
				vec3 diffuseOut = (diffuse * inColor) / (dist * dist);

				float scaleFactor = scaleValue / bands;

				diffuseOut = diffuseOut * lightIntensity;
				diffuseOut = floor(diffuseOut * bands) * scaleFactor;

				float edge = (dot(V, inNormal) < 0.2) ? 0.0 : 1.0;

				result = ((u_AmbientCol * u_AmbientStrength) + (ambient + specular) * attenuation) * diffuseOut;		
			}else{
				result = (
					(u_AmbientCol * u_AmbientStrength) + // global ambient light
					(ambient + diffuse + specular) * attenuation // light factors from our single light
					) * inColor; // Object color
			}			
			break;
	}

	if (u_TextureColor){
		result*=textureColor.rgb;
		frag_color = vec4(result, textureColor.a);
	}else{
		frag_color = vec4(result, 1.0);
	}
}