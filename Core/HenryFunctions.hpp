#pragma once

#ifndef HENRYFUNCTIONS_HPP 
#define HENRYFUNCTIONS_HPP

#include "Engine\Math\Vec3.hpp"

#include <stdlib.h>
#include <vector>

namespace Henry
{
inline float degree2radians(float degrees){ return degrees*0.0174f;  /* PI / 180 */ }
inline float radians2degrees(float radians){ return radians*57.2957795f;}
inline float getRandomPercent(){ const int MOD_MASK_2047 = 2047; float result = (float)((rand() & MOD_MASK_2047) * 0.0009765625f); return result > 1.0f ? result - 1.0f : result; /* 1 / 1024 */ };
inline float random(float min,float max){ max = max < min ? min : max; return min + (max - min) * getRandomPercent(); };
inline int random(int min,int max) { max = max < min ? min : max; return (int)( min + (max - min) * getRandomPercent()); };
inline float getDegree(Vec2f direction){ return atan2(direction.y, direction.x) * 57.295779f; };
inline float clamp(float value, float min, float max){ return value > max ? max : (value < min ? min : value); };
void circleTable(double **sint,double **cost,const int n);

bool ComputeSurfaceTangentsAtVertex(
	Vec3f& surfaceTangentAtVertex_out,
	Vec3f& surfaceBitangentAtVertex_out,
	const Vec3f& normalAtThisVertex,
	const Vec3f& positionOfThisVertex,
	const Vec2f& texCoordsOfThisVertex,
	const Vec3f& positionOfPreviousAdjacentVertex,
	const Vec2f& texCoordsOfPreviousAdjacentVertex,
	const Vec3f& positionOfNextAdjacentVertex,
	const Vec2f& texCoordsOfNextAdjacentVertex );

inline Vec3f ComputeSurfaceNormalAtTriangle(const Vec3f& vertexA , const Vec3f& vertexB , const Vec3f& vertexC)
{
	Vec3f vertexAToB = vertexB-vertexA;
	Vec3f vertexBToC = vertexC-vertexB;
	return vertexAToB.crossProductWith(vertexBToC);
}


void DebuggerPrintf( const char* messageFormat, ... );


template <class T>
void deleteVectorOfPointer(std::vector<T*>& pointerVector)
{
	std::vector<T*>::iterator iter = pointerVector.begin();
	while(iter != pointerVector.end())
	{
		T* temp = *iter;
		iter = pointerVector.erase(iter);
		if(temp)
		{
			delete temp;
			temp = nullptr;
		}
	}
}


void RotateAroundPoint2D(Vec2f& pointToRotate, Vec2f rotateAround, float const radians);


inline float Vector2Radians2D(Vec2f const vector2) { return atan2(vector2.y, vector2.x); };


};

#endif