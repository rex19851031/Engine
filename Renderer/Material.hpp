#pragma once

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <string>
#include <vector>
#include <map>

#include "Texture.hpp"
#include "OpenGLShader.hpp"

namespace Henry
{

struct ShaderProgram
{
	ShaderProgram(){ programID = 0; fragment = 0; vertex = 0;};
	ShaderProgram(int programID,OpenGLShader* fragment,OpenGLShader* vertex) : programID(programID) , fragment(fragment) , vertex(vertex) {};
	int programID;
	OpenGLShader* fragment;
	OpenGLShader* vertex;
};

class Material
{
public:
	Material();
	Material(ShaderProgram shaderProgram);
	~Material(void);
	void SendUniform(const char* name , Vec2f data);
	void SendUniform(const char* name , Vec3f data);
	void SendUniform(const char* name , int data);
	void SendUniform(const char* name , float data);
	void SendUniform(const char* name , RGBA data);
	void SendUniform(const char* name , Matrix4 data);
	void SendUniform(const char* name , std::vector<Vec3f> data);
	void SendUniform(const char* name , std::vector<RGBA> data);
	void SendUniform(const char* name , std::vector<float> data);
	void Activate();
	ShaderProgram m_shaderProgram;
	
	Texture* m_emissiveTexture;
	Texture* m_specularTexture;
	Texture* m_normalTexture;
	Texture* m_diffuseTexture;
};

};

#endif