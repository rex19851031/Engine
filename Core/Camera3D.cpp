#include "Camera3D.hpp"
#include "Engine\Core\HenryFunctions.hpp"

#include <cmath>

#define UNUSED(x) (void)(x);

namespace Henry
{

Camera3D::Camera3D(InputSystem* input) : m_input(input)
{
	m_position = Vec3f(3.f,3.f,3.f);
	m_orientation.yawDegreesAboutZ = 225.f;
	m_orientation.pitchDegreesAboutY = 30.f;	
}


Camera3D::~Camera3D(void)
{
}


void Camera3D::SetPosition(const Vec3f pos)
{

}


void Camera3D::Update(float deltaSeconds)
{
	//UNUSED(deltaSeconds);
	m_position += (m_movementVector * deltaSeconds);
	m_movementVector = Vec3f( 0.f, 0.f, 0.f );

	//UpdateVectors();
}


void Camera3D::UpdateVectors()
{
	float cameraYawRadians = degree2radians(m_orientation.yawDegreesAboutZ);
	float cameraPitchRadians = degree2radians(m_orientation.pitchDegreesAboutY);
	Vec3f cameraForwardVector = Vec3f(cos(cameraYawRadians) * cos(cameraPitchRadians), sin(cameraYawRadians) * cos(cameraPitchRadians), -sin(cameraPitchRadians));
	m_forwardVector = cameraForwardVector;
	m_rightVector = m_forwardVector.crossProductWith(Vec3f(0, 0, 1));
	m_upVector = m_forwardVector.crossProductWith(m_rightVector);
}


};