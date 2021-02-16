#include "Graphics/Post/PostEffect.h"

/*

Referenced Code from SpriteLib

Justification: Used the code as a base to understand how to apply multiple framebuffers and the base bloom code

Modifications: I modified all the code for pushing the shaders into the Shader::Sptr Vector,
			   I also changed all the variables names to show the change from the Spritelib to the Otter framework,
			   using the PostEffect class created through the tutorial

*/

class BloomEffect : public PostEffect
{
public:

	void Init(unsigned width, unsigned height) override;

	void ApplyEffect(PostEffect* buffer) override;

	void Reshape(unsigned width, unsigned height) override;

	//Getters
	float GetDownscale() const;
	float GetThreshold() const;
	unsigned GetPasses() const;

	//Setters
	void SetDownscale(float downscale);
	void SetThreshold(float threshold);
	void SetPasses(unsigned passes);

private:

	float _downscale = 2.f;
	float _threshold = 0.01f;
	unsigned _passes = 10;
	glm::vec2 _BlurValues;
};