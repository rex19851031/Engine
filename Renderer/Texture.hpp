#pragma once

#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <stdlib.h>
#include <string>
#include <map>

#include "Engine\Math\Vec2.hpp"


namespace Henry
{

typedef Vec2<int> TextureSize;

class Texture
{
public:
	Texture::Texture( const std::string& imageFilePath );
	Texture::Texture( const std::string& imageFilePath , const std::string& zipFilePath);
	~Texture(void);
	Texture* Texture::GetTextureByName( const std::string& imageFilePath );
	Texture* Texture::CreateOrGetTexture( const std::string& imageFilePath );
	Texture* Texture::CreateOrGetTexture( const std::string& imageFilePath , const std::string& zipFilePath);
	static std::map< std::string, Texture* > s_textureRegistry;
	TextureSize m_size;
	int m_textureID;
};

};

#endif

