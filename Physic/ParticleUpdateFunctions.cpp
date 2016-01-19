#include "ParticleUpdateFunctions.hpp"

namespace Henry
{

#define MOD_MASK_1024 1023
Vec3f windForce(1.0f,1.0f,1.0f);

void ParticleUpdateFunctions::LeafUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	Vec3f force = Vec3f(((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f));
	particle.color = RGBA(1.0f,1.0f,1.0f,0.5f);
	particle.position += (particle.velocity) * deltaTime;
	force += (particle.velocity - windForce) * - particle.c;
	
	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		float offset = 512.f;
		particle.velocity = Vec3f((((rand() & MOD_MASK_1024) - offset) * 0.001f),(((rand() & MOD_MASK_1024) - offset) * 0.001f),(((rand() & MOD_MASK_1024) - offset) * 0.001f)) * 50.0f + emitter.velocity;
		particle.color = RGBA(1.0f,1.0f,1.0f,0.5f);
	}
}


void ParticleUpdateFunctions::ExplosionUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	if(particle.surviveTime == emitter.survivalTime)
	{
		float yawRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		float pitchRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		particle.velocity = Vec3f( cos(yawRadians) * cos(pitchRadians) , sin(yawRadians) * cos(pitchRadians) , -sin(pitchRadians)) * emitter.velocity;
		particle.color = RGBA(1.0f,0.2f,0.2f,0.8f);
	}

	Vec3f force;

	particle.position += particle.velocity * deltaTime;
	force += particle.velocity * -particle.c;
	
	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		float yawRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		float pitchRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		particle.velocity = Vec3f( cos(yawRadians) * cos(pitchRadians) , sin(yawRadians) * cos(pitchRadians) , -sin(pitchRadians)) * emitter.velocity;
		particle.color = RGBA(1.0f,0.2f,0.2f,0.8f);
	}
}


void ParticleUpdateFunctions::FountainUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	if(particle.surviveTime == emitter.survivalTime)
	{
		float yawRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		particle.velocity.x = 2.0f*cos(yawRadians);
		particle.velocity.y = 2.0f*sin(yawRadians);
		particle.color = RGBA(0.0f,0.8f,1.0f,1.0f);
	}

	Vec3f force;// = Vec3f(10.0f*((rand() & MOD_MASK_1024) * 0.002f - 1.0f),10.0f*((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f));

	particle.position += particle.velocity * deltaTime;
	force += particle.velocity * -particle.c + Vec3f(0.0f,0.0f,-9.8f*particle.mass);

	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		particle.velocity = emitter.velocity;
		particle.color = RGBA(0.0f,0.8f,1.0f,1.0f);
	}
}


void ParticleUpdateFunctions::DebrisUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	if(particle.surviveTime == emitter.survivalTime)
	{
		float yawRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		particle.velocity.x = 2.0f*cos(yawRadians);
		particle.velocity.y = 2.0f*sin(yawRadians);
		particle.color = RGBA(0.0f,0.8f,1.0f,1.0f);
	}

	Vec3f force;

	particle.position += particle.velocity * deltaTime;
	force += particle.velocity * -particle.c + Vec3f(0.0f,0.0f,-9.8f*particle.mass);

	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.position.z < -2.0f)
	{
		particle.velocity.z *= -0.5f;
	}

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		particle.velocity = emitter.velocity;
		particle.color = RGBA(0.0f,0.8f,1.0f,1.0f);
	}
}


void ParticleUpdateFunctions::FireworkUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	if(particle.surviveTime == emitter.survivalTime)
	{
		float yawRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		float pitchRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		particle.velocity = Vec3f( cos(yawRadians) * cos(pitchRadians) , sin(yawRadians) * cos(pitchRadians) , -sin(pitchRadians)) * emitter.velocity;
		particle.color = RGBA((rand() & MOD_MASK_1024) * 0.001f,(rand() & MOD_MASK_1024) * 0.001f,(rand() & MOD_MASK_1024) * 0.001f,1.0f);
	}
	Vec3f force;
	particle.color = RGBA((rand() & MOD_MASK_1024) * 0.001f,(rand() & MOD_MASK_1024) * 0.001f,(rand() & MOD_MASK_1024) * 0.001f,1.0f);
	particle.size *= 0.6f+(rand() & MOD_MASK_1024) * 0.0006f;

	particle.position += particle.velocity * deltaTime;
	force += particle.velocity * -particle.c + Vec3f(0.0f,0.0f,-(rand() & MOD_MASK_1024) * 0.008f * particle.mass);

	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		float yawRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		float pitchRadians = (rand() & MOD_MASK_1024) * 0.00628f;
		particle.velocity = Vec3f( cos(yawRadians) * cos(pitchRadians) , sin(yawRadians) * cos(pitchRadians) , -sin(pitchRadians)) * emitter.velocity;
		particle.color = RGBA((rand() & MOD_MASK_1024) * 0.001f,(rand() & MOD_MASK_1024) * 0.001f,(rand() & MOD_MASK_1024) * 0.001f,1.0f);
	}
}


void ParticleUpdateFunctions::SnowUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	Vec3f force = Vec3f(((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f),0.0f);
	particle.color = RGBA(1.0f,1.0f,1.0f,0.5f);
	particle.position += particle.velocity * deltaTime;
	force += particle.velocity * -particle.c;

	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		particle.velocity = emitter.velocity;
		particle.color = RGBA(1.0f,1.0f,1.0f,0.5f);
	}
}


void ParticleUpdateFunctions::FireBallUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	Vec3f force = Vec3f(((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f)) * 0.0f;
	particle.color = RGBA(1.0f,1.0f,1.0f,1.0f);
	particle.position += particle.velocity * deltaTime;
	force += (particle.velocity - windForce) * - particle.c;

	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		particle.velocity = emitter.velocity;
		particle.color = RGBA(1.0f,1.0f,1.0f,1.0f);
	}
}


void ParticleUpdateFunctions::SmokeUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	Vec3f force = Vec3f(((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f)) * 200;
	particle.color = RGBA(0.5f,0.5f,0.5f,0.1f);
	particle.position += particle.velocity * deltaTime;
	force += (particle.velocity - windForce) * - particle.c;

	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		particle.velocity = emitter.velocity;
		particle.color = RGBA(0.5f,0.5f,0.5f,0.1f);
	}
}


void ParticleUpdateFunctions::BurstUpdate(Emitter& emitter,Particle& particle,float deltaTime)
{
	Vec3f force = Vec3f(((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f),((rand() & MOD_MASK_1024) * 0.002f - 1.0f)) * 100;
	particle.color = RGBA(0.5f,0.5f,0.5f,0.5f);
	particle.position += particle.velocity * deltaTime;
	force += (particle.velocity - windForce) * - particle.c;

	float massOverOne = 1.0f/particle.mass;
	Vec3f acceleration = force * massOverOne;

	particle.velocity += acceleration * deltaTime;
	particle.surviveTime -= deltaTime;

	if(particle.surviveTime < 0)
	{
		Vec3f positionDifference = emitter.maxPos - emitter.minPos;
		Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
		particle.position = emitter.minPos + (positionDifference * randomCoefficient);
		particle.surviveTime = emitter.survivalTime;
		particle.velocity = emitter.velocity;
		particle.color = RGBA(0.5f,0.5f,0.5f,0.5f);
	}
}

};