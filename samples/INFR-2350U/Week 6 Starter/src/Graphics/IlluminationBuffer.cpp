#include "IlluminationBuffer.h"

void IlluminationBuffer::Init(unsigned width, unsigned height)
{
	//Ambient + Illum Buffer
	int index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	_buffers[index]->AddDepthTarget();
	_buffers[index]->Init(width, height);

<<<<<<< HEAD
	//Composite Buffer
	int index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	_buffers[index]->AddDepthTarget();
	_buffers[index]->Init(width, height);

	//Illum Buffer
=======
	//IllumBuf 1
>>>>>>> master
	index = int(_buffers.size());
	_buffers.push_back(new Framebuffer());
	_buffers[index]->AddColorTarget(GL_RGBA8);
	_buffers[index]->AddDepthTarget();
	_buffers[index]->Init(width, height);

<<<<<<< HEAD

	//check if the shader is initialized
	//Load in the shader

	index= int(_shaders.size());

=======
	//Loads the directional shader
	index = int(_shaders.size());
>>>>>>> master
	_shaders.push_back(Shader::Create());
	_shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_shaders[index]->LoadShaderPartFromFile("shaders/gBuffer_directional_frag.glsl", GL_FRAGMENT_SHADER);
	_shaders[index]->Link();

<<<<<<< HEAD

	index= int(_shaders.size());

=======
	//Loads the point light shader
	index = int(_shaders.size());
>>>>>>> master
	_shaders.push_back(Shader::Create());
	_shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
	_shaders[index]->LoadShaderPartFromFile("shaders/gBuffer_ambient_frag.glsl", GL_FRAGMENT_SHADER);
	_shaders[index]->Link();

<<<<<<< HEAD
	_sunBuffer.AllocateMemory(sizeof(DirectionalLight));

	if (_sunEnabled){
	
=======
	//Allocate the memory for the buffers
	_sunBuffer.AllocateMemory(sizeof(DirectionalLight));

	if (_sunEnabled)
	{
		//We are just assuming there's a sun
>>>>>>> master
		_sunBuffer.SendData(reinterpret_cast<void*>(&_sun), sizeof(DirectionalLight));
	}

	PostEffect::Init(width, height);
<<<<<<< HEAD

=======
>>>>>>> master
}

void IlluminationBuffer::ApplyEffect(GBuffer* gBuffer)
{
<<<<<<< HEAD
	_sunBuffer.SendData(reinterpret_cast<void*>(&_sun), sizeof(DirectionalLight));

	if (_sunEnabled)
	{
		_shaders[Lights::DIRECTIONAL]->Bind();
		_shaders[Lights::DIRECTIONAL]->SetUniformMatrix("u_LightSPaceMatrix", _lightSpaceViewProj);
		_shaders[Lights::DIRECTIONAL]->SetUniform("u_CamPos", _camPos);

		_sunBuffer.Bind(0);

		gBuffer->BindLighting();

		_buffers[1]->RenderToFSQ();

		gBuffer->UnbindLighting();

		_sunBuffer.Unbind(0);

		_shaders[Lights::DIRECTIONAL]->UnBind();

	}

	_shaders[Lights::AMBIENT]->Bind();

	_sunBuffer.Bind(0);

	gBuffer->BindLighting();

	_buffers[1]->BindColorAsTexture(0,4);
	_buffers[0]->BindColorAsTexture(0,5);
	
=======
	glDisable(GL_DEPTH_TEST);
	
	//If the sun is enabled
	if (_sunEnabled)
	{
		//Binds the directional light shader
		_shaders[Lights::DIRECTIONAL]->Bind();
		_shaders[Lights::DIRECTIONAL]->SetUniformMatrix("u_LightSpaceMatrix", _lightSpaceViewProj);
		_shaders[Lights::DIRECTIONAL]->SetUniform("u_CamPos", _camPos);

		//Send the directional light data to the uniform buffer
		_sunBuffer.SendData(reinterpret_cast<void*>(&_sun), sizeof(DirectionalLight));
		_sunBuffer.Bind(0);

		gBuffer->BindLighting();

		//Binds and draws to the illumination buffer
		_buffers[1]->RenderToFSQ();

		gBuffer->UnbindLighting();

		//Unbinds the uniform buffer
		_sunBuffer.Unbind(0);

		//Unbind directional light shader
		_shaders[Lights::DIRECTIONAL]->UnBind();
	}

	//By the end whatever is readBuf is our final buffer
	_shaders[Lights::AMBIENT]->Bind();
	
	//Send the directional light data to the uniform buffer
	_sunBuffer.SendData(reinterpret_cast<void*>(&_sun), sizeof(DirectionalLight));
	_sunBuffer.Bind(0);

	gBuffer->BindLighting();
	_buffers[1]->BindColorAsTexture(0, 4);
	_buffers[0]->BindColorAsTexture(0, 5);

>>>>>>> master
	_buffers[0]->RenderToFSQ();

	_buffers[0]->UnbindTexture(5);
	_buffers[1]->UnbindTexture(4);
<<<<<<< HEAD

	gBuffer->UnbindLighting();
	
=======
	gBuffer->UnbindLighting();

	//Unbinds the uniform buffer
>>>>>>> master
	_sunBuffer.Unbind(0);

	_shaders[Lights::AMBIENT]->UnBind();

<<<<<<< HEAD
=======
	glEnable(GL_DEPTH_TEST);
>>>>>>> master
}

void IlluminationBuffer::DrawIllumBuffer()
{
<<<<<<< HEAD
	_shaders[_shaders.size()-1]->Bind();
	
	_buffers[1]->BindColorAsTexture(0,0);
=======
	_shaders[_shaders.size() - 1]->Bind();

	_buffers[1]->BindColorAsTexture(0, 0);
>>>>>>> master

	Framebuffer::DrawFullscreenQuad();

	_buffers[1]->UnbindTexture(0);
<<<<<<< HEAD
	
	_shaders[_shaders.size()-1]->UnBind();
=======

	_shaders[_shaders.size() - 1]->UnBind();
>>>>>>> master
}

void IlluminationBuffer::SetLightSpaceViewProj(glm::mat4 lightSpaceViewProj)
{
	_lightSpaceViewProj = lightSpaceViewProj;
}

void IlluminationBuffer::SetCamPos(glm::vec3 camPos)
{
	_camPos = camPos;
}

DirectionalLight& IlluminationBuffer::GetSunRef()
{
	return _sun;
}

void IlluminationBuffer::SetSun(DirectionalLight newSun)
{
<<<<<<< HEAD
	_sun =	newSun;
=======
	_sun = newSun;
>>>>>>> master
}

void IlluminationBuffer::SetSun(glm::vec4 lightDir, glm::vec4 lightCol)
{
	_sun._lightDirection = lightDir;
	_sun._lightCol = lightCol;
}

void IlluminationBuffer::EnableSun(bool enabled)
{
	_sunEnabled = enabled;
}
