#pragma once

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Engine\Math\Vec3.hpp"
#include "Engine\Input\InputSystem.hpp"

namespace Henry
{

struct Orientation
{
	float rollDegreesAboutX;
	float pitchDegreesAboutY;
	float yawDegreesAboutZ;
	Orientation(float roll,float pitch,float yaw):
		rollDegreesAboutX(roll),
		pitchDegreesAboutY(pitch),
		yawDegreesAboutZ(yaw){};
	Orientation():
		rollDegreesAboutX(0),
		pitchDegreesAboutY(0),
		yawDegreesAboutZ(0){};
};

class Camera3D
{
public:
	Camera3D(InputSystem* input);
	~Camera3D(void);
	void SetPosition(const Vec3f pos);
	void Update(float deltaSeconds);
	void UpdateVectors();

public:
	float m_speed;
	Vec3f m_position;
	Vec3f m_forwardVector;
	Vec3f m_rightVector;
	Vec3f m_upVector;
	Vec3f m_movementVector;
	Orientation m_orientation;

private:
	InputSystem* m_input;
};

};

#endif