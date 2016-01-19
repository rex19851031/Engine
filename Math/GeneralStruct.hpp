#pragma once

#ifndef GENERALSTRUCT_HPP 
#define	GENERALSTRUCT_HPP

#include "Engine\Core\VertexStruct.hpp"

namespace Henry
{

struct Light
{
	float degrees;
	float lightEdgeSmoothness;
	float innerRadius;
	float outerRadius;
	float ambient;
	RGBA color;
	Vec3f position;
	Vec3f direction;
};


struct FBO
{
	FBO(){ ID=0; colorTextureID=0; depthTextureID=0; }
	int ID;
	int colorTextureID;
	int depthTextureID;
};


struct Circle2D
{
	Circle2D(){};
	Circle2D(Vec2f _center,float _radius) : center(_center) , radius(_radius){ };
	Vec2f center;
	float radius;
};


struct AABB2
{
	AABB2(){};
	AABB2(Vec2f _min,Vec2f _max){ min = _min; max = _max; };
	Vec2f min;
	Vec2f max;
	bool isOverlap( const AABB2* aabb ) {	if(max.x < aabb->min.x || min.x > aabb->max.x) return false;
											if(max.y < aabb->min.y || min.y > aabb->max.y) return false;
											return true; };
};

};

#endif