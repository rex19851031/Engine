#ifndef SPRITESANIMATESYSTEM_HPP
#define SPRITESANIMATESYSTEM_HPP

#include "Engine/Renderer/Sprites.hpp"
#include "Engine/Math/GeneralStruct.hpp"

namespace Henry
{
	class SpritesAnimateSystem
	{
	public:
		SpritesAnimateSystem(const char* texturePath, Vec2i spriteDimentions, const char* spritesName);
		~SpritesAnimateSystem();
		void RegistryAnimation(const char* spritesName, const char* animationName, AABB2 indexInterval, float timePerFrame);

	private:

	};
}


#endif // !SPRITESANIMATESYSTEM_HPP
