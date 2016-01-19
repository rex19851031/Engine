#pragma once

#ifndef PARTICLESYSTEM_HPP 
#define PARTICLESYSTEM_HPP

#include "Engine\Core\VertexStruct.hpp"
#include <vector>

namespace Henry
{

enum EMITTER_SHAPE { BOX = 0 , CONE , SPHERE };

struct Particle
{
	float mass;
	float k;
	float c;
	Vec3f position;
	Vec3f velocity;
	float surviveTime;
	float size;
	RGBA color;
};

struct Emitter;
typedef void(ParticleUpdate)(Emitter& emitter,Particle& particle,float deltaTime);

struct Emitter
{
	Emitter(Vec3f _minPos,Vec3f _maxPos,Vec3f _velocity,ParticleUpdate func,EMITTER_SHAPE _shape = BOX,float _radius = 0.0f) : minPos(_minPos), maxPos(_maxPos), updateFunc(func), velocity(_velocity), shape(_shape), radius(_radius){};
	Emitter(){};
	EMITTER_SHAPE shape;
	float radius;
	float survivalTime;
	Vec3f minPos;
	Vec3f maxPos;
	Vec3f velocity;
	Vec3f gravity;
	ParticleUpdate* updateFunc;
};

class ParticleSystem
{
public:
	ParticleSystem(Emitter emitter, Particle particle , int num, float time, bool launchTogether = false);
	ParticleSystem(void);
	~ParticleSystem(void);
	void Draw();
	void Draw2D();
	void Update(float delteTime, bool hasRandomSpawn = false);
	Emitter m_emitter;
	Vec3f m_velocity;
	float m_numOfParticles;
	float m_surviveTime;
	size_t m_maxNum;
	bool m_launchTogether;
	EMITTER_SHAPE m_emitterShape;
private:
	void SetInitialPosition(Particle* particle);
	void AddNewParticleToList();
	void AddNewParticleToListWithRandomDirectionAndForce();
	Particle m_particleTemplate;
	std::vector<Particle> m_particles;
	std::vector<Vertex_PCT> m_vertices;
};

};

#endif