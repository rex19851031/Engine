#pragma once

#ifndef ZIPHELPER_HPP
#define ZIPHELPER_HPP

#include <string>
#include <map>

namespace Henry
{

class ZipHelper
{
public:
	ZipHelper();
	~ZipHelper();
	static void LoadZipToMap(const std::string& zipFilePath, const std::string& password);
	static const unsigned char* GetContentInZip(const std::string& zipFilePath, const std::string& pathInZip, const std::string& password, int* bufferLength, bool* success);
	static std::map< std::string , void* > s_zipMap;
	static void* GetZip(const std::string& zipFilePath);
};

};

#endif