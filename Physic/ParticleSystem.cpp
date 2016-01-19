#include "Engine\Physic\ParticleSystem.hpp"
#include "Engine\Renderer\OpenGLRenderer.hpp"
#include <math.h>


namespace Henry
{

#define MOD_MASK_1024 1023

ParticleSystem::ParticleSystem(Emitter emitter, Particle particle, int num, float time, bool launchTogether)
{
	m_emitter = emitter;
	m_particles.reserve((int)num);
	m_surviveTime = time;
	m_emitter.survivalTime = time;
	m_velocity = emitter.velocity;
	m_maxNum = num;
	m_particleTemplate = particle;
	particle.surviveTime = time;
	m_launchTogether = launchTogether;

	if(launchTogether)
	{
		for(int index= 0; index < num; index++)
		{
			Particle p(m_particleTemplate);
			m_particles.push_back(p);
		}
	}
}


ParticleSystem::ParticleSystem(void)
{
	
}


ParticleSystem::~ParticleSystem(void)
{

}


void ParticleSystem::Update(float deltaTime, bool hasRandomSpawn)
{
	if(hasRandomSpawn)
	{
		AddNewParticleToListWithRandomDirectionAndForce();
	}
	else
	{
		AddNewParticleToList();
	}
	
	Vec3f force;

	std::vector<Particle>::iterator it = m_particles.begin();
	while( it != m_particles.end() )
	{
		Particle& particle = (*it);
		m_emitter.updateFunc(m_emitter,particle,deltaTime);
		it++;
	}
}


void ParticleSystem::Draw()
{
	std::vector<Vertex_PosColor> vertices;
	std::vector<Particle>::iterator it = m_particles.begin();
	while(it != m_particles.end())
	{
		Particle particle = *it;
		Vertex_PosColor vps[8];
		vps[0].position = Vec3f(particle.position.x - particle.size , particle.position.y - particle.size , particle.position.z - particle.size);
		vps[1].position = Vec3f(particle.position.x + particle.size , particle.position.y + particle.size , particle.position.z + particle.size);
		vps[2].position = Vec3f(particle.position.x - particle.size , particle.position.y - particle.size , particle.position.z + particle.size);
		vps[3].position = Vec3f(particle.position.x + particle.size , particle.position.y + particle.size , particle.position.z - particle.size);
		vps[4].position = Vec3f(particle.position.x + particle.size , particle.position.y - particle.size , particle.position.z + particle.size);
		vps[5].position = Vec3f(particle.position.x - particle.size , particle.position.y + particle.size , particle.position.z - particle.size);
		vps[6].position = Vec3f(particle.position.x - particle.size , particle.position.y + particle.size , particle.position.z + particle.size);
		vps[7].position = Vec3f(particle.position.x + particle.size , particle.position.y - particle.size , particle.position.z - particle.size);

		vps[0].color = particle.color;
		vps[1].color = particle.color;
		vps[2].color = particle.color;
		vps[3].color = particle.color;
		vps[4].color = particle.color;
		vps[5].color = particle.color;
		vps[6].color = particle.color;
		vps[7].color = particle.color;

		vertices.push_back(vps[0]);
		vertices.push_back(vps[1]);
		vertices.push_back(vps[2]);
		vertices.push_back(vps[3]);
		vertices.push_back(vps[4]);
		vertices.push_back(vps[5]);
		vertices.push_back(vps[6]);
		vertices.push_back(vps[7]);
		it++;
	}

	OpenGLRenderer::LineWidth(5.0f);
	OpenGLRenderer::DrawVertexWithVertexArray(vertices,OpenGLRenderer::SHAPE_LINES);
}


void ParticleSystem::AddNewParticleToList()
{
	if(m_particles.size() < m_maxNum)
	{
		Particle p(m_particleTemplate);
		SetInitialPosition(&p);
		p.velocity = m_velocity;
		p.surviveTime = m_surviveTime;
		m_particles.push_back(p);
	}

	if(m_particles.size() > m_maxNum)
	{
		std::vector<Particle> empty;
		m_particles = empty;
	}
}

void ParticleSystem::AddNewParticleToListWithRandomDirectionAndForce()
{
	if(m_particles.size() < m_maxNum)
	{
		Particle p(m_particleTemplate);
		SetInitialPosition(&p);
		float offset = 512.f;
		p.velocity = Vec3f((((rand() & MOD_MASK_1024) - offset) * 0.001f),(((rand() & MOD_MASK_1024) - offset) * 0.001f),(((rand() & MOD_MASK_1024) - offset) * 0.001f)) * 50.0f + m_velocity;
		p.surviveTime = m_surviveTime;
		m_particles.push_back(p);
	}

	if(m_particles.size() > m_maxNum)
	{
		std::vector<Particle> empty;
		m_particles = empty;
	}
}

void ParticleSystem::SetInitialPosition(Particle* particle)
{
	Vec3f positionDifference = m_emitter.maxPos - m_emitter.minPos;
	Vec3f randomCoefficient = Vec3f(((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f),((rand() & MOD_MASK_1024) * 0.001f));
	particle->position = m_emitter.minPos + (positionDifference * randomCoefficient);
}


void ParticleSystem::Draw2D()
{
	if(m_particles.size() * 4 != m_particles.size())
		m_vertices.reserve(m_particles.size() * 4);
	std::vector<Particle>::iterator it = m_particles.begin();
	while(it != m_particles.end())
	{
		Particle particle = *it;
		Vertex_PCT vpct[4];
		vpct[0].position = Vec3f(particle.position.x - particle.size , particle.position.y - particle.size , 0.0f );
		vpct[1].position = Vec3f(particle.position.x + particle.size , particle.position.y - particle.size , 0.0f );
		vpct[2].position = Vec3f(particle.position.x + particle.size , particle.position.y + particle.size , 0.0f );
		vpct[3].position = Vec3f(particle.position.x - particle.size , particle.position.y + particle.size , 0.0f );
		
		vpct[0].texCoords = Vec2f(0,1);
		vpct[1].texCoords = Vec2f(1,1);
		vpct[2].texCoords = Vec2f(1,0);
		vpct[3].texCoords = Vec2f(0,0);

		vpct[0].color = particle.color;
		vpct[1].color = particle.color;
		vpct[2].color = particle.color;
		vpct[3].color = particle.color;

		m_vertices.push_back(vpct[0]);
		m_vertices.push_back(vpct[1]);
		m_vertices.push_back(vpct[2]);
		m_vertices.push_back(vpct[3]);
		it++;
	}

	OpenGLRenderer::DrawVertexWithVertexArray2D(m_vertices,OpenGLRenderer::SHAPE_QUADS);
	
	std::vector<Vertex_PCT> empty;
	m_vertices = empty;
}

};