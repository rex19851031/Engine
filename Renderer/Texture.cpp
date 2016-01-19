//---------------------------------------------------------------------------
#include "Texture.hpp"
#include "OpenGLRenderer.hpp"
#include "Engine\Parsing\ZipUtils\unzip.h"

#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"


namespace Henry
{

#define STATIC // Do-nothing indicator that method/member is static in class definition


//---------------------------------------------------------------------------
STATIC std::map< std::string, Texture* > Texture::s_textureRegistry;


//---------------------------------------------------------------------------
Texture::Texture( const std::string& imageFilePath )
	: m_textureID( 0 )
	, m_size( 0, 0 )
{
	Texture* texture = CreateOrGetTexture(imageFilePath.c_str());
	m_textureID = texture->m_textureID;
}


//---------------------------------------------------------------------------
Texture::Texture(const std::string& imageFilePath , const std::string& zipFilePath)
	: m_textureID( 0 )
	, m_size( 0, 0 )
{
	Texture* texture = CreateOrGetTexture(imageFilePath.c_str() , zipFilePath);
	m_textureID = texture->m_textureID;
}


//---------------------------------------------------------------------------
Texture::~Texture()
{
	
}


//---------------------------------------------------------------------------
// Returns a pointer to the already-loaded texture of a given image file,
//	or nullptr if no such texture/image has been loaded.
//
STATIC Texture* Texture::GetTextureByName( const std::string& imageFilePath )
{
	// Todo: you write this
	Texture* texture = nullptr;
	std::map<std::string, Texture* >::iterator it = s_textureRegistry.find(imageFilePath.c_str());
	if(it != s_textureRegistry.end())
		texture = it->second;
	return texture;
}


//---------------------------------------------------------------------------
// Finds the named Texture among the registry of those already loaded; if
//	found, returns that Texture*.  If not, attempts to load that texture,
//	and returns a Texture* just created (or nullptr if unable to load file).
//
STATIC Texture* Texture::CreateOrGetTexture( const std::string& imageFilePath )
{
	// Todo: you write this
	Texture* texture = nullptr;
	texture = GetTextureByName(imageFilePath.c_str());
	if(texture == nullptr)
	{
		m_textureID = OpenGLRenderer::LoadTexture( imageFilePath , m_size );
		texture = this;
		if(m_textureID != -1)
		{
			s_textureRegistry[imageFilePath.c_str()] = this;
		}
	}
	return texture;
}


//---------------------------------------------------------------------------
STATIC Texture* Texture::CreateOrGetTexture(const std::string& imageFilePath , const std::string& zipFilePath)
{
	Texture* texture = nullptr;
	texture = GetTextureByName(imageFilePath.c_str());
	if(texture == nullptr)
	{
		m_textureID = OpenGLRenderer::LoadTexture( imageFilePath , m_size , zipFilePath );
		texture = this;
		if(m_textureID != -1)
		{
			s_textureRegistry[imageFilePath.c_str()] = this;
		}
	}
	return texture;
}


};