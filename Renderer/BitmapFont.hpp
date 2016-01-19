#pragma once

#ifndef BMPFONT_HPP 
#define BMPFONT_HPP

#include "Engine\Parsing\TinyXML\tinyxml.h"
#include "Engine\Math\GeneralStruct.hpp"
#include "Texture.hpp"

#include <map>
#include <vector>
#include <sstream>

namespace Henry
{

struct GlyphMetaData
{
	AABB2 m_textCoords;
	float m_ttfA;
	float m_ttfB;
	float m_ttfC;
};


enum BuildInFont { NONE = 0 , ARIAL , ARIAL_NORMAL , BITFONT , BOOKANTIQUA , BUXTON , BUXTON_NORMAL };


class BitmapFont
{
public:
	BitmapFont(char* fontDocPath , bool* success = new bool, bool autoParsing = true);
	BitmapFont(char* fontDocPath , char* zipFilePath , bool* success , bool autoParsing = true);
	BitmapFont(BuildInFont font = ARIAL_NORMAL , bool autoParsing = true);
	~BitmapFont(void);
	void Draw(const std::string& sentence , const Vec2f& position , const float& fontHeight , const RGBA& color , const Vec2i canvasCoord , ...);
	float GetWidthOfText(const std::string& sentence, float& fontHeight, ...);
	void ParsingXmlDataAndLoadTexture(BuildInFont bf = NONE , const std::string& zipFilePath = "");

public:
	TiXmlDocument* m_metaDoc;
	Texture* m_glyphSheet;
	std::map<BuildInFont,std::ostringstream> m_buildInFontXML;
	std::map<BuildInFont,int> m_buildInFontTextureID;
	std::map<int,GlyphMetaData> m_glyphData;
	std::vector<float> m_widthTable;
	int m_fontHeight;

private:
	void Initialize();
	void BindBuildInTexture(BuildInFont bf);
	std::string ConvertArgument(const char*message , va_list args);
};

};

#endif