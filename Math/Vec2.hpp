#pragma once

#ifndef VEC2_HPP
#define VEC2_HPP

namespace Henry
{

template <class T>
class Vec2
{
public:
	Vec2(void){ x = 0; y = 0; }
	Vec2(const T x,const T y){this->x = x; this->y = y;}
	~Vec2(void){}
	Vec2<T> operator+(const Vec2<T>& rhs)const{ Vec2<T> lhs=*this; lhs.x+=rhs.x; lhs.y+=rhs.y; return lhs;}
	Vec2<T> operator+(const T& rhs)const{ Vec2<T> lhs=*this; lhs.x+=rhs; lhs.y+=rhs; return lhs;}
	Vec2<T> operator-(const Vec2<T>& rhs)const{ Vec2<T> lhs=*this; lhs.x-=rhs.x; lhs.y-=rhs.y; return lhs;}
	Vec2<T> operator-(const T& rhs)const{ Vec2<T> lhs=*this; lhs.x-=rhs; lhs.y-=rhs; return lhs;}
	Vec2<T> operator*(const Vec2<T>& rhs)const{ Vec2<T> lhs=*this; lhs.x*=rhs.x; lhs.y*=rhs.y; return lhs;}
	Vec2<T> operator*(const T& rhs)const{ Vec2<T> lhs=*this; lhs.x*=rhs; lhs.y*=rhs; return lhs;}
	Vec2<T> operator/(const Vec2<T>& rhs)const{ Vec2<T> lhs=*this; lhs.x/=rhs.x; lhs.y/=rhs.y; return lhs;}
	Vec2<T> operator/(const T& rhs)const{ Vec2<T> lhs=*this; lhs.x/=rhs; lhs.y/=rhs; return lhs;}
	Vec2<T> operator*=(const T& rhs){ x*=rhs; y*=rhs; return *this;}
	Vec2<T> operator+=(const Vec2<T>& rhs){ x+=rhs.x; y+=rhs.y; return *this;}
	Vec2<T> operator-=(const Vec2<T>& rhs){ x-=rhs.x; y-=rhs.y; return *this;}

	bool operator==(const Vec2<T>& rhs) const { Vec2<T> lhs=*this; return (rhs.x==lhs.x && rhs.y == lhs.y); }
	bool operator!=(const Vec2<T>& rhs) const { Vec2<T> lhs=*this; return (rhs.x!=lhs.x || rhs.y != lhs.y); }
	void setAsUnitVectorOfDegree(T degrees){ x=cos(degrees * 0.0174f); y=sin(degrees * 0.0174f); }
	Vec2<T> getUnitVector();
	T length();
	T lengthSquare();
	T normalize();
	T x,y;
};

typedef Vec2<float> Vec2f;
typedef Vec2<double> Vec2d;
typedef Vec2<int> Vec2i;

template <class T>
T Vec2<T>::length()
{
	T length = sqrt( x*x + y*y);
	return length;
}

template <class T>
T Vec2<T>::lengthSquare()
{
	T lengthSquare = x*x + y*y;
	return lengthSquare;
}

template <class T>
Vec2<T> Vec2<T>::getUnitVector()
{
	T magnetude = length();
	x/=magnetude;
	y/=magnetude;
	return *this;
}

template <class T>
T Vec2<T>::normalize()
{
	T magnetude = length();

	if(magnetude > 0)
	{
		x = x / magnetude;
		y = y / magnetude;
	}
	else
	{
		x *= 0;
		y *= 0;
		return 0;
	}

	return magnetude;
}

};

#endif