#include "IlluminationBuffer.h"

void IlluminationBuffer::Init(unsigned width, unsigned height)
{

}

void IlluminationBuffer::ApplyEffect(GBuffer* gBuffer)
{
	
}

void IlluminationBuffer::DrawIllumBuffer()
{
	
}

void IlluminationBuffer::SetLightSpaceViewProj(glm::mat4 lightSpaceViewProj)
{
	
}

void IlluminationBuffer::SetCamPos(glm::vec3 camPos)
{
	
}

DirectionalLight& IlluminationBuffer::GetSunRef()
{
	return _sun;
}

void IlluminationBuffer::SetSun(DirectionalLight newSun)
{

}

void IlluminationBuffer::SetSun(glm::vec4 lightDir, glm::vec4 lightCol)
{
	
}

void IlluminationBuffer::EnableSun(bool enabled)
{
	
}
