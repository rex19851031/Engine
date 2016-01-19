#pragma once

#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "Texture.hpp"
#include "Engine/Math/Vec3.hpp"

namespace Henry
{

struct AnimationRange
{
	AnimationRange() { startIndex = 0; endIndex = 0; };
	AnimationRange(int start, int end) : startIndex(start), endIndex(end) { };
	int startIndex;
	int endIndex;
	bool operator==(const AnimationRange& rhs) const { AnimationRange lhs = *this; return (rhs.startIndex == lhs.startIndex && rhs.endIndex == lhs.endIndex); }
	bool operator!=(const AnimationRange& rhs) const { AnimationRange lhs = *this; return (rhs.startIndex != lhs.startIndex || rhs.endIndex != lhs.endIndex); }
};


class Sprites
{
public:
	Sprites(const char* texturePath , Vec2i spriteDimentions , float timePerFrame = 0.0f , AnimationRange animationIndexRange = AnimationRange());
	Sprites(Texture* texture, Vec2i spriteDimentions, float timePerFrame = 0.0f, AnimationRange animationIndexRange = AnimationRange());
	~Sprites(void);
	Vec2f GetCoordinateByIndex(const int index);
	Vec2f GetCoordinateByXY(const int x , const int y);
	void SetRenderSize(const Vec2f size);
	void Update();
	void Draw();
	void Draw(const int index);
	void Draw(const Vec2i coordinate);

	void SetAnimationRange( const AnimationRange& animationIndexRange );
	//void SetAnimationRange( const Vec2i startCoord, const Vec2i endCoord );

	Vec2f m_sizeOfEachSprites;
	int m_numOfSprites;
	int m_currentSpritesIndex;
	Texture* m_texture;

	Vec2f m_halfRenderSize;
	Vec2i m_spriteDimentions;
	Vec3f m_position;

	float m_deltaTime;
	AnimationRange m_currentIndexRange;
	int m_currentIndex;
	float m_startingTimeOfCurrentImageFrame;
	bool m_loopMultiple;
};

};

#endif