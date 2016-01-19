#pragma once

#ifndef TINYXMLHELPER_HPP
#define TINYXMLHELPER_HPP

#include "Engine\Parsing\TinyXML\tinyxml.h"

namespace Henry
{

class TinyXMLHelper
{
public:
	TinyXMLHelper();
	~TinyXMLHelper();
	void LoadFile(const char* filePath);
	
public:
	static int ToInt(const TiXmlElement* elem, const char* attributeName);
	static float ToFloat(const TiXmlElement* elem, const char* attributeName);
	static char* ToString(const TiXmlElement* elem, const char* attributeName);
	static bool IsValid(const TiXmlElement* elem, const char* attributeName);

public:
	TiXmlDocument* m_metaDoc;
	TiXmlElement* m_root;
};

};

#endif