#pragma once

#ifndef MATRIX4X4_HPP 
#define MATRIX4X4_HPP

#include "Vec3.hpp"

namespace Henry
{

class Matrix4
{
public:
	Matrix4(void);
	~Matrix4(void);
	Matrix4(float* matrix);
	void ApplyTransformMatrix(const float* matrix);
	void ApplyTransform(float x,float y,float z);
	void ApplyRotate(float degrees,float x,float y,float z);
	void ApplyScale(float x,float y,float z);
	void LoadIdentity();
	void Transpose();
	Matrix4 GetTransformMatrix(const float* matrix);
	float m_matrix[16];
};

};

#endif