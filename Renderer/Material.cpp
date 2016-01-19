#include "Material.hpp"
#include "OpenGLRenderer.hpp"
#include "Engine\Core\Time.hpp"

namespace Henry
{

Material::Material()
{
	m_diffuseTexture = nullptr;
	m_emissiveTexture = nullptr;
	m_specularTexture = nullptr;
	m_normalTexture = nullptr;
}


Material::Material(ShaderProgram shaderProgram)
{
	m_shaderProgram = shaderProgram;

	bool wasSuccessful;
	OpenGLRenderer::AttachShader(m_shaderProgram.programID,m_shaderProgram.vertex->m_shaderID);
	OpenGLRenderer::AttachShader(m_shaderProgram.programID,m_shaderProgram.fragment->m_shaderID);
	OpenGLRenderer::LinkProgram(m_shaderProgram.programID);
	OpenGLRenderer::GetProgramIV(m_shaderProgram.programID,OpenGLRenderer::LINK_STATUS,wasSuccessful);

	m_diffuseTexture = nullptr;
	m_emissiveTexture = nullptr;
	m_specularTexture = nullptr;
	m_normalTexture = nullptr;
}


Material::~Material(void)
{
	delete m_diffuseTexture;
	delete m_emissiveTexture;
	delete m_specularTexture;
	delete m_normalTexture;
}


void Material::SendUniform(const char* name , Vec2f data)
{
	int location = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , name );
	OpenGLRenderer::SetUniform( location , data );
}


void Material::SendUniform(const char* name , Vec3f data)
{
	int location = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , name );
	OpenGLRenderer::SetUniform( location , data );
}


void Material::SendUniform(const char* name , float data)
{
	int location = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , name );
	OpenGLRenderer::SetUniform( location , data );
}


void Material::SendUniform(const char* name, int data)
{
	int location = OpenGLRenderer::GetUniformLocation(m_shaderProgram.programID, name);
	OpenGLRenderer::SetUniform(location, data);
}


void Material::SendUniform(const char* name , RGBA data)
{
	int location = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , name);
	OpenGLRenderer::SetUniform( location , data);
}


void Material::SendUniform(const char* name , Matrix4 data)
{
	int location = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , name );
	OpenGLRenderer::SetUniform( location , data );
}


void Material::SendUniform(const char* name, std::vector<Vec3f> data)
{
	int location = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , name );
	OpenGLRenderer::SetUniform( location , data );
}


void Material::SendUniform(const char* name, std::vector<RGBA> data)
{
	int location = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , name );
	OpenGLRenderer::SetUniform( location , data );
}


void Material::SendUniform(const char* name, std::vector<float> data)
{
	int location = OpenGLRenderer::GetUniformLocation(m_shaderProgram.programID, name);
	OpenGLRenderer::SetUniform(location, data);
}


void Material::Activate()
{
	OpenGLRenderer::BindTexture(m_diffuseTexture,m_emissiveTexture,m_normalTexture,m_specularTexture);
	OpenGLRenderer::UseProgram( m_shaderProgram.programID );

	int currentMatrixLocation = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID , "u_worldToClipMatrix" );
	int diffuseMapUniformLocation = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID, "u_diffuseMap" );
	int normalMapUniformLocation = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID, "u_normalMap" );
	int specularMapUniformLocation = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID, "u_specularMap" );
	int emissiveMapUniformLocation = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID, "u_emissiveMap" );
	int backgroundMapUniformLocation = OpenGLRenderer::GetUniformLocation( m_shaderProgram.programID, "u_backgroundMap" );

	OpenGLRenderer::SetUniform( currentMatrixLocation, OpenGLRenderer::WorldToClipMatrixStack.back() );
	OpenGLRenderer::SetUniform( diffuseMapUniformLocation, 0  );
	OpenGLRenderer::SetUniform( normalMapUniformLocation, 1   );
	OpenGLRenderer::SetUniform( specularMapUniformLocation, 2  );
	OpenGLRenderer::SetUniform( emissiveMapUniformLocation, 3  );
	OpenGLRenderer::SetUniform( backgroundMapUniformLocation, 4 );
}

};