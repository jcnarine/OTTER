#pragma once

#include "Graphics/Framebuffer.h"
#include "Shader.h"

class PostEffect
{
public:
	virtual void Init( unsigned width, unsigned height);
	virtual void ApplyEffect(PostEffect* previousBuffer);
	virtual void DrawToScreen();

	//reshapes the buffer
	virtual void Reshape(unsigned width, unsigned height);

	//clear the buffer
	void Clear();

	void Unload();

	void BindBuffer(int index);
	void UnbindBuffer();

	void BindColorAsTexture(int index, int colorBuffer, int textureSlot);
	void BindDepthAsTexture(int index, int textureSlot);
	void UnbindTexture(int textureSlot);

	void BindShader(int index);
	void UnbindShader();

protected:

	std::vector<Framebuffer*>_buffers;
	std::vector<Shader::sptr>_shaders;
};