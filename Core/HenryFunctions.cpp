#include "Engine/Core/HenryFunctions.hpp"

#include <Windows.h>
#include <iostream>

namespace Henry
{

bool ComputeSurfaceTangentsAtVertex(	Vec3f& surfaceTangentAtVertex_out, 
										Vec3f& surfaceBitangentAtVertex_out, 
										const Vec3f& normalAtThisVertex, 
										const Vec3f& positionOfThisVertex, 
										const Vec2f& texCoordsOfThisVertex, 
										const Vec3f& positionOfPreviousAdjacentVertex, 
										const Vec2f& texCoordsOfPreviousAdjacentVertex, 
										const Vec3f& positionOfNextAdjacentVertex, 
										const Vec2f& texCoordsOfNextAdjacentVertex )
{
	{
		//-----------------------------------------------------------------------------------------------
		// Returns TRUE if the Bitangent is "right handed" with respect to the Normal and Tangent;
		// i.e. if [Tangent,Bitangent,Normal] is a right-handed basis.  Typically this bool is
		// converted to a +/- 1.0 and passed into a vertex shader as the Tangent's "w" coordinate.
		// The Bitangent is then computed within the vertex shader using cross product and "w" to
		// disambiguate between which of the two possible perpendiculars to use for the Bitangent.
		//

		Vec3f vecToPrevious	= positionOfPreviousAdjacentVertex - positionOfThisVertex;
		Vec3f vecToNext		= positionOfNextAdjacentVertex - positionOfThisVertex;

		Vec2f texToPrevious	= texCoordsOfPreviousAdjacentVertex - texCoordsOfThisVertex;
		Vec2f texToNext		= texCoordsOfNextAdjacentVertex - texCoordsOfThisVertex;

		float determinant = ((texToPrevious.x * texToNext.y) - (texToNext.x * texToPrevious.y));

		Vec3f uDirectionInWorldSpace(	
			(texToNext.y * vecToPrevious.x - texToPrevious.y * vecToNext.x),
			(texToNext.y * vecToPrevious.y - texToPrevious.y * vecToNext.y),
			(texToNext.y * vecToPrevious.z - texToPrevious.y * vecToNext.z)
			);

		Vec3f vDirectionInWorldSpace(
			(texToPrevious.x * vecToNext.x - texToNext.x * vecToPrevious.x),
			(texToPrevious.x * vecToNext.y - texToNext.x * vecToPrevious.y),
			(texToPrevious.x * vecToNext.z - texToNext.x * vecToPrevious.z)
			);

		float invDeterminant = 1.0f / determinant;
		uDirectionInWorldSpace = uDirectionInWorldSpace * invDeterminant;
		vDirectionInWorldSpace = vDirectionInWorldSpace * invDeterminant;

		surfaceTangentAtVertex_out = uDirectionInWorldSpace;
		surfaceBitangentAtVertex_out = vDirectionInWorldSpace;// * (-1); // NOTE: You should remove this minus sign if your V texture coordinates are using the opposite convention of mine!

		// NOTE: You don't need the following code, unless you intend to reconstitute the Bitangent vector inside the vertex shader (and not pass it in as a vertex attribute)
		Vec3f naturalBitangentFromCross = normalAtThisVertex.crossProductWith( surfaceTangentAtVertex_out );
		float computedBitangentDotNaturalBitangent = surfaceBitangentAtVertex_out.dotProductWith( naturalBitangentFromCross );
		return( computedBitangentDotNaturalBitangent >= 0.f );
	}
}


void circleTable(double **sint,double **cost,const int n)
{
	float M_PI = 3.1415926f;
	int i;

	/* Table size, the sign of n flips the circle direction */

	const int size = abs(n);

	/* Determine the angle between samples */

	const double angle = 2*M_PI/(double)n;

	/* Allocate memory for n samples, plus duplicate of first entry at the end */

	*sint = (double *) calloc(sizeof(double), size+1);
	*cost = (double *) calloc(sizeof(double), size+1);

	/* Bail out if memory allocation fails, fgError never returns */

	if (!(*sint) || !(*cost))
	{
		free(*sint);
		free(*cost);
		//fgError("Failed to allocate memory in circleTable");
	}

	/* Compute cos and sin around the circle */

	for (i=0; i<size; i++)
	{
		(*sint)[i] = sin(angle*i);
		(*cost)[i] = cos(angle*i);
	}

	/* Last sample is duplicate of the first */

	(*sint)[size] = (*sint)[0];
	(*cost)[size] = (*cost)[0];
}


void DebuggerPrintf( const char* messageFormat, ... )
{
	{
		const int MESSAGE_MAX_LENGTH = 2048;
		char messageLiteral[ MESSAGE_MAX_LENGTH ];
		va_list variableArgumentList;
		va_start( variableArgumentList, messageFormat );
		vsnprintf_s( messageLiteral, MESSAGE_MAX_LENGTH, _TRUNCATE, messageFormat, variableArgumentList );
		va_end( variableArgumentList );
		messageLiteral[ MESSAGE_MAX_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

// #if defined( PLATFORM_WINDOWS )
// 		if( IsDebuggerAvailable() )
// 		{
// 			OutputDebugStringA( messageLiteral );
// 		}
// #endif
		OutputDebugStringA(messageLiteral);
		std::cout << messageLiteral;
	}
}


void RotateAroundPoint2D(Vec2f& pointToRotate, Vec2f rotateAround, float const radians)
{
	pointToRotate.x -= rotateAround.x;
	pointToRotate.y -= rotateAround.y;
	
	float newX = pointToRotate.x * cos(radians) - pointToRotate.y * sin(radians);
	float newY = pointToRotate.x * sin(radians) + pointToRotate.y * cos(radians);

	pointToRotate.x = newX + rotateAround.x;
	pointToRotate.y = newY + rotateAround.y;
}


};