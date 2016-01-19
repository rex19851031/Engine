#ifndef QUATERNIONHPP
#define QUATERNIONHPP

#define PI	3.14159265358979323846

namespace Henry
{ 

class Quaternion
{
public:
	Quaternion();
	~Quaternion();

	void CreateMatrix(float *pMatrix);
	void CreateFromAxisAngle(const float &in_x,
							 const float &in_y,
							 const float &in_z,
							 const float &in_degrees);

	Quaternion operator *(const Quaternion &q);

private:
	float x,y,z,w;
};

}

#endif