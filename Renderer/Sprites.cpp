#include "Sprites.hpp"

#include <vector>
#include "Engine\Core\VertexStruct.hpp"
#include "Engine\Renderer\OpenGLRenderer.hpp"
#include "Engine\Core\Time.hpp"

namespace Henry
{


Sprites::Sprites(const char* texturePath , Vec2i spriteDimensions , float timePerFrame , AnimationRange animationIndexRange )
{
	m_texture = new Texture(texturePath);
	m_spriteDimentions = spriteDimensions;
	m_sizeOfEachSprites.x = 1.0f / m_spriteDimentions.x;
	m_sizeOfEachSprites.y = 1.0f / m_spriteDimentions.y;
	m_numOfSprites = m_spriteDimentions.x * m_spriteDimentions.y;
	m_currentSpritesIndex = 0;
	m_halfRenderSize = Vec2f(1.0f,1.0f);

	m_deltaTime = timePerFrame;
	m_startingTimeOfCurrentImageFrame = (float) GetCurrentTimeSeconds();
	m_position = Vec3f(0.f, 0.f, 0.f);

	m_currentIndex = animationIndexRange.startIndex;
	m_currentIndexRange = animationIndexRange;
}


Sprites::Sprites(Texture* texture, Vec2i spriteDimentions , float timePerFrame, AnimationRange animationIndexRange)
{
	m_texture = texture;
	m_spriteDimentions = spriteDimentions;
	m_sizeOfEachSprites.x = 1.0f / m_spriteDimentions.x;
	m_sizeOfEachSprites.y = 1.0f / m_spriteDimentions.y;
	m_numOfSprites = m_spriteDimentions.x * m_spriteDimentions.y;
	m_currentSpritesIndex = 0;
	m_halfRenderSize = Vec2f(1.0f,1.0f);

	m_deltaTime = timePerFrame;
	m_currentIndex = animationIndexRange.startIndex;
	m_currentIndexRange = animationIndexRange;
	m_startingTimeOfCurrentImageFrame = (float) GetCurrentTimeSeconds();
	m_position = Vec3f(0.f, 0.f, 0.f);
}


Sprites::~Sprites(void)
{
	if (m_texture)
	{
		delete m_texture;
		m_texture = nullptr;
	}
}


void Sprites::SetRenderSize(const Vec2f size)
{
	m_halfRenderSize.x = size.x * 0.5f;
	m_halfRenderSize.y = size.y * 0.5f;
}


Vec2f Sprites::GetCoordinateByIndex(int index)
{
	Vec2f coordinate = Vec2f((index%m_spriteDimentions.x) * m_sizeOfEachSprites.x,(index/m_spriteDimentions.x) * m_sizeOfEachSprites.y);
	return coordinate;
}


void Sprites::SetAnimationRange( const AnimationRange& animationIndexRange )
{
	if(animationIndexRange != m_currentIndexRange)
	{
		m_currentIndex = animationIndexRange.startIndex;
		m_currentIndexRange = animationIndexRange;
	}
}

/*
void Sprites::SetAnimationRange(const Vec2i startCoord, const Vec2i endCoord)
{

}
*/

void Sprites::Update()
{
	float currentTime = (float) GetCurrentTimeSeconds();
	if( currentTime > (m_startingTimeOfCurrentImageFrame + m_deltaTime) )
	{
		m_startingTimeOfCurrentImageFrame = currentTime;
		++m_currentIndex;
		if( m_currentIndex > m_currentIndexRange.endIndex )
		{
			m_currentIndex = m_currentIndexRange.startIndex;
		}
	}
}


void Sprites::Draw()
{
	Draw( m_currentIndex );
}


void Sprites::Draw(const int index)
{
	OpenGLRenderer::BindTexture(m_texture);

	AABB2 aabb;
	aabb.min = GetCoordinateByIndex(index);
	aabb.max = aabb.min + m_sizeOfEachSprites;

	Vertex_PCT vertices[4];
	vertices[0].position = Vec3f(-m_halfRenderSize.x + m_position.x, m_halfRenderSize.y + m_position.y, m_position.z);
	vertices[0].texCoords = aabb.min;
	vertices[0].color = RGBA();

	vertices[1].position = Vec3f(-m_halfRenderSize.x + m_position.x, -m_halfRenderSize.y + m_position.y, m_position.z);
	vertices[1].texCoords = Vec2f(aabb.min.x,aabb.max.y);
	vertices[1].color = RGBA();

	vertices[2].position = Vec3f(m_halfRenderSize.x + m_position.x, -m_halfRenderSize.y + m_position.y, m_position.z);
	vertices[2].texCoords = aabb.max;
	vertices[2].color = RGBA();

	vertices[3].position = Vec3f(m_halfRenderSize.x + m_position.x, m_halfRenderSize.y + m_position.y, m_position.z);
	vertices[3].texCoords = Vec2f(aabb.max.x,aabb.min.y);
	vertices[3].color = RGBA();

	OpenGLRenderer::DrawVertexWithVertexArray(vertices,4,OpenGLRenderer::SHAPE_QUADS);
}


/*top-left is (0,0)*/
void Sprites::Draw(const Vec2i coordinate)
{
	OpenGLRenderer::BindTexture(m_texture);

	AABB2 aabb;
	aabb.min = Vec2f(coordinate.x * m_sizeOfEachSprites.x, coordinate.y * m_sizeOfEachSprites.y);
	aabb.max = aabb.min + m_sizeOfEachSprites;

	Vertex_PCT vertices[4];
	vertices[0].position = Vec3f(-m_halfRenderSize.x + m_position.x, m_halfRenderSize.y + m_position.y, 0.0f);
	vertices[0].texCoords = aabb.min;
	vertices[0].color = RGBA();

	vertices[1].position = Vec3f(-m_halfRenderSize.x + m_position.x, -m_halfRenderSize.y + m_position.y, 0.0f);
	vertices[1].texCoords = Vec2f(aabb.min.x, aabb.max.y);
	vertices[1].color = RGBA();

	vertices[2].position = Vec3f(m_halfRenderSize.x + m_position.x, -m_halfRenderSize.y + m_position.y, 0.0f);
	vertices[2].texCoords = aabb.max;
	vertices[2].color = RGBA();

	vertices[3].position = Vec3f(m_halfRenderSize.x + m_position.x, m_halfRenderSize.y + m_position.y, 0.0f);
	vertices[3].texCoords = Vec2f(aabb.max.x, aabb.min.y);
	vertices[3].color = RGBA();

	OpenGLRenderer::DrawVertexWithVertexArray(vertices, 4, OpenGLRenderer::SHAPE_QUADS);
}


};