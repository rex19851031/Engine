#pragma once

#ifndef VEC3_HPP 
#define VEC3_HPP

#include <cmath>
#include "Vec2.hpp"


namespace Henry
{

template<class T>
class Vec3
{
public:
	Vec3(void){ x = 0; y = 0; z = 0;};
	Vec3(T ix,T iy,T iz): x(ix), y(iy), z(iz){};
	~Vec3(void){};
	
	template<typename S>
	Vec3<T> operator*(S rhs) const { return Vec3<T>( x * rhs , y * rhs , z * rhs ); } ;
	template<typename S>
	Vec3<T> operator-(S rhs) const { return Vec3<T>( x-rhs,y-rhs,z-rhs); };
	template<typename S>
	Vec3<T> operator+(S rhs) const { return Vec3<T>( x+rhs,y+rhs,z+rhs); };
	
	template<typename S>
	Vec3<T> operator-(Vec3<S> rhs) const { return Vec3<T>( x-rhs.x,y-rhs.y,z-rhs.z); };
	template<typename S>
	Vec3<T> operator*(Vec3<S> rhs) const { return Vec3<T>( x*rhs.x,y*rhs.y,z*rhs.z); };
	template<typename S>
	Vec3<T> operator+(Vec3<S> rhs) const { return Vec3<T>( x+rhs.x,y+rhs.y,z+rhs.z); };

	bool operator!=(Vec3<T> rhs) { return ((x != rhs.x) || (y != rhs.y) || (y != rhs.y)); };
	bool operator==(Vec3<T> rhs) { return ((x == rhs.x) && (y == rhs.y) && (y == rhs.y)); };
	void operator+=(Vec3<T> rhs) { x += rhs.x; y += rhs.y; z += rhs.z; };
	void operator-=(Vec3<T> rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; };
	void operator*=(Vec3<T> rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; };
	void operator*=(T rhs) { x *= rhs; y *= rhs; z *= rhs; };

	void normalize();
	Vec3<T> crossProductWith(Vec3<T> rhs) const;
	T dotProductWith(Vec3<T> rhs) const;
	T lengthSquare(Vec3<T> rhs) const;

	T x;
	T y;
	T z;
};

typedef Vec3<int> Vec3i;
typedef Vec3<float> Vec3f;
typedef Vec3<double> Vec3d;

template <class T>
Vec3<T> Vec3<T>::crossProductWith(Vec3<T> rhs) const
{
	Vec3<T> product;

	product.x = (y * rhs.z) - (z * rhs.y);
	product.y = (z * rhs.x) - (x * rhs.z);
	product.z = (x * rhs.y) - (y * rhs.x);

	return product;
}


template <class T>
void Vec3<T>::normalize()
{
	T length = sqrt( x*x + y*y + z*z);
	
	if (length == 0)
		return;

	x = x / length;
	y = y / length;
	z = z / length;
}


template <class T>
T Vec3<T>::dotProductWith(Vec3<T> rhs) const
{
	T dot = x * rhs.x + y * rhs.y + z * rhs.z;
	return dot;
}


template <class T>
T Vec3<T>::lengthSquare(Vec3<T> rhs) const
{
	T square = (x - rhs.x) * (x - rhs.x) + (y - rhs.y) * (y - rhs.y) + (z - rhs.z) * (z - rhs.z);
	return square;
}

};

#endif