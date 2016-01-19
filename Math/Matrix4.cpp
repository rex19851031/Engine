#include "Matrix4.hpp"

#include "Engine\Core\HenryFunctions.hpp"

#include <math.h>

namespace Henry
{

Matrix4::Matrix4(void)
{
	m_matrix[0] = 1.0f; m_matrix[4] = 0.0f; m_matrix[8] = 0.0f;		m_matrix[12] = 0.0f;
	m_matrix[1] = 0.0f; m_matrix[5] = 1.0f; m_matrix[9] = 0.0f;		m_matrix[13] = 0.0f;
	m_matrix[2] = 0.0f; m_matrix[6] = 0.0f; m_matrix[10] = 1.0f;	m_matrix[14] = 0.0f;
	m_matrix[3] = 0.0f; m_matrix[7] = 0.0f; m_matrix[11] = 0.0f;	m_matrix[15] = 1.0f;
}


Matrix4::~Matrix4(void)
{
}


Matrix4::Matrix4(float* matrix)
{
	for(int index = 0; index < 16; index++)
	{
		m_matrix[index] = matrix[index];
	}
}


void Matrix4::ApplyTransformMatrix(const float* n)
{
	float answer[16];
	answer[0]	= m_matrix[0] * n[0] + m_matrix[4] * n[1] + m_matrix[8]  * n[2] + m_matrix[12] * n[3];	
	answer[1]	= m_matrix[1] * n[0] + m_matrix[5] * n[1] + m_matrix[9]  * n[2] + m_matrix[13] * n[3];
	answer[2]	= m_matrix[2] * n[0] + m_matrix[6] * n[1] + m_matrix[10] * n[2] + m_matrix[14] * n[3];
	answer[3]	= m_matrix[3] * n[0] + m_matrix[7] * n[1] + m_matrix[11] * n[2] + m_matrix[15] * n[3];
	
	answer[4]	= m_matrix[0] * n[4] + m_matrix[4] * n[5] + m_matrix[8]  * n[6] + m_matrix[12] * n[7];	
	answer[5]	= m_matrix[1] * n[4] + m_matrix[5] * n[5] + m_matrix[9]  * n[6] + m_matrix[13] * n[7];	
	answer[6]	= m_matrix[2] * n[4] + m_matrix[6] * n[5] + m_matrix[10] * n[6] + m_matrix[14] * n[7];	
	answer[7]	= m_matrix[3] * n[4] + m_matrix[7] * n[5] + m_matrix[11] * n[6] + m_matrix[15] * n[7];
	
	answer[8]	= m_matrix[0] * n[8] + m_matrix[4] * n[9] + m_matrix[8]  * n[10] + m_matrix[12] * n[11];	
	answer[9]	= m_matrix[1] * n[8] + m_matrix[5] * n[9] + m_matrix[9]  * n[10] + m_matrix[13] * n[11];	
	answer[10]	= m_matrix[2] * n[8] + m_matrix[6] * n[9] + m_matrix[10] * n[10] + m_matrix[14] * n[11];	
	answer[11]	= m_matrix[3] * n[8] + m_matrix[7] * n[9] + m_matrix[11] * n[10] + m_matrix[15] * n[11];
	
	answer[12]	= m_matrix[0] * n[12] + m_matrix[4] * n[13] + m_matrix[8]  * n[14] + m_matrix[12] * n[15];	
	answer[13]	= m_matrix[1] * n[12] + m_matrix[5] * n[13] + m_matrix[9]  * n[14] + m_matrix[13] * n[15];	
	answer[14]	= m_matrix[2] * n[12] + m_matrix[6] * n[13] + m_matrix[10] * n[14] + m_matrix[14] * n[15];	
	answer[15]	= m_matrix[3] * n[12] + m_matrix[7] * n[13] + m_matrix[11] * n[14] + m_matrix[15] * n[15];

	for(int index=0; index < 16; index++)
	{
		m_matrix[index] = answer[index];
	}
}


Matrix4 Matrix4::GetTransformMatrix(const float* n)
{
	float answer[16];
	float* c = this->m_matrix;

	answer[0]	= c[0] * n[0]	+ c[4] * n[1]	+ c[8] * n[2]	+ c[12] * n[3];		
	answer[4]	= c[0] * n[4]	+ c[4] * n[5]	+ c[8] * n[6]	+ c[12] * n[7];	
	answer[8]	= c[0] * n[8]	+ c[4] * n[9]	+ c[8] * n[10]	+ c[12] * n[11];	
	answer[12]	= c[0] * n[12]	+ c[4] * n[13]	+ c[8] * n[14]	+ c[12] * n[15];

	answer[1]	= c[1] * n[0]	+ c[5] * n[1]	+ c[9] * n[2]	+ c[13] * n[3];		
	answer[5]	= c[1] * n[4]	+ c[5] * n[5]	+ c[9] * n[6]	+ c[13] * n[7];		
	answer[9]	= c[1] * n[8]	+ c[5] * n[9]	+ c[9] * n[10]	+ c[13] * n[11];	
	answer[13]	= c[1] * n[12]	+ c[5] * n[13]	+ c[9] * n[14]	+ c[13] * n[15];

	answer[2]	= c[2] * n[0]	+ c[6] * n[1]	+ c[10] * n[2]	+ c[14] * n[3];		
	answer[6]	= c[2] * n[4]	+ c[6] * n[5]	+ c[10] * n[6]	+ c[14] * n[7];		
	answer[10]	= c[2] * n[8]	+ c[6] * n[9]	+ c[10] * n[10]	+ c[14] * n[11];	
	answer[14]	= c[2] * n[12]	+ c[6] * n[13]	+ c[10] * n[14]	+ c[14] * n[15];

	answer[3]	= c[3] * n[0]	+ c[7] * n[1]	+ c[11] * n[2]	+ c[15] * n[3];		
	answer[7]	= c[3] * n[4]	+ c[7] * n[5]	+ c[11] * n[6]	+ c[15] * n[7];		
	answer[11]	= c[3] * n[8]	+ c[7] * n[9]	+ c[11] * n[10]	+ c[15] * n[11];	
	answer[15]	= c[3] * n[12]	+ c[7] * n[13]	+ c[11] * n[14]	+ c[15] * n[15];

	return Matrix4(answer);
}


void Matrix4::ApplyScale(float x, float y, float z)
{
	float scale[16] = {	x	 , 0.0f , 0.0f , 0.0f,
						0.0f , y	, 0.0f , 0.0f,
						0.0f , 0.0f , z	   , 0.0f,
						0.0f , 0.0f , 0.0f , 1.0f };

	ApplyTransformMatrix(scale);
}


void Matrix4::ApplyTransform(float x,float y,float z)
{
	float transform[16] = {	1.0f , 0.0f , 0.0f , 0.0f,
							0.0f , 1.0f , 0.0f , 0.0f,
							0.0f , 0.0f , 1.0f , 0.0f,
							x , y , z , 1.0f};

	ApplyTransformMatrix(transform);
}


void Matrix4::ApplyRotate(float degrees,float x,float y,float z)
{
	float radians = degree2radians(degrees);
	float _cos = cos(radians);
	float _sin = sin(radians);

	Vec3f axis = Vec3f(x,y,z);
	axis.normalize();

	float c1 = 1.0f - _cos;
	float rotate[16];

	rotate[0] = axis.x * axis.x * c1 + _cos;
	rotate[1] = axis.x * axis.y * c1 + axis.z * _sin;
	rotate[2] = axis.x * axis.z * c1 - axis.y * _sin;
	rotate[3] = 0.0f;
	rotate[4] = axis.y * axis.x * c1 - axis.z * _sin;
	rotate[5] = axis.y * axis.y * c1 + _cos;
	rotate[6] = axis.y * axis.z * c1 + axis.x * _sin;
	rotate[7] = 0.0f;
	rotate[8] = axis.z * axis.x * c1 + axis.y * _sin;
	rotate[9] = axis.z * axis.y * c1 - axis.x * _sin;
	rotate[10]= axis.z * axis.z * c1 + _cos;
	rotate[11] = 0.0f;
	rotate[12] = 0.0f;
	rotate[13] = 0.0f;
	rotate[14] = 0.0f;
	rotate[15] = 1.0f;

	ApplyTransformMatrix(rotate);
}


void Matrix4::LoadIdentity()
{
	m_matrix[0] = 1.0f; m_matrix[4] = 0.0f; m_matrix[8] = 0.0f;		m_matrix[12] = 0.0f;
	m_matrix[1] = 0.0f; m_matrix[5] = 1.0f; m_matrix[9] = 0.0f;		m_matrix[13] = 0.0f;
	m_matrix[2] = 0.0f; m_matrix[6] = 0.0f; m_matrix[10] = 1.0f;	m_matrix[14] = 0.0f;
	m_matrix[3] = 0.0f; m_matrix[7] = 0.0f; m_matrix[11] = 0.0f;	m_matrix[15] = 1.0f;
}


void Matrix4::Transpose()
{
	std::swap(m_matrix[1], m_matrix[4]);
	std::swap(m_matrix[2], m_matrix[8]);
	std::swap(m_matrix[3], m_matrix[12]);
	std::swap(m_matrix[6], m_matrix[9]);
	std::swap(m_matrix[7], m_matrix[13]);
	std::swap(m_matrix[11], m_matrix[14]);
}


};