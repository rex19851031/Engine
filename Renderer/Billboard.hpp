#ifndef BILLBOARD_HPP 
#define BILLBOARD_HPP

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Matrix4.hpp"

namespace Henry
{

	class Billboard
{
public:
	Billboard();
	~Billboard();
	static void AxisAligned(Vec3f BillboardPosition, Vec3f CameraPosition, Matrix4& matrix);
	static void WorldAligned(Vec3f BillboardPosition, Vec3f CameraPosition, Matrix4& matrix);
	static void LookAt(Vec3f ObjectPosition, Vec3f TargetPosition, Matrix4& matrix);
};

}

#endif