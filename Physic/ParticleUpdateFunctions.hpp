#pragma once

#ifndef PARTICLEUPDATEFUNCTIONS_HPP 
#define PARTICLEUPDATEFUNCTIONS_HPP

#include "ParticleSystem.hpp"

namespace Henry
{

class ParticleUpdateFunctions
{
public:
	static void LeafUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void ExplosionUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void FountainUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void DebrisUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void FireworkUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void SnowUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void FireBallUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void SmokeUpdate(Emitter& emitter,Particle& particle,float deltaTime);
	static void BurstUpdate(Emitter& emitter,Particle& particle,float deltaTime);
};

};

#endif