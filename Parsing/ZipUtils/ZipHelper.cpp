#include "ZipHelper.hpp"

#include <string>
#include <Windows.h>
#include <atlconv.h>
#include <algorithm>

#include "Engine\Parsing\ZipUtils\zip.h"
#include "Engine\Parsing\ZipUtils\unzip.h"


namespace Henry
{

std::map< std::string , void* > ZipHelper::s_zipMap;


ZipHelper::ZipHelper(void) 
{
}


ZipHelper::~ZipHelper(void)
{
	std::map< std::string , void* >::iterator it = s_zipMap.begin();
	for( ; it != s_zipMap.end(); ++it )
		CloseZip((HZIP)it->second);
}


void ZipHelper::LoadZipToMap(const std::string& zipFilePath , const std::string& password)
{
	std::string lowerZipPath = zipFilePath;
	std::transform(lowerZipPath.begin(), lowerZipPath.end(), lowerZipPath.begin(), ::tolower);

	USES_CONVERSION;
	TCHAR* zipPath = A2T(zipFilePath.c_str());
	HZIP zipFile = OpenZip(zipPath, password.c_str());
	s_zipMap[lowerZipPath] = zipFile;
}


const unsigned char* ZipHelper::GetContentInZip(const std::string& zipFilePath , const std::string& pathInZip , const std::string& password , int* bufferLength , bool* success)
{
	*success = true;
	HZIP zipFile = (HZIP)GetZip(zipFilePath);
	if(zipFile)
	{
		ZIPENTRY ze;
		int index;
		USES_CONVERSION;
		TCHAR* lpString = pathInZip.substr(0,2) == "./" ? A2T(pathInZip.substr(2).c_str()) : A2T(pathInZip.c_str());
		FindZipItem(zipFile, lpString, true, &index, &ze);
		if(index == -1)
		{
			std::string errorInfo(pathInZip);
			errorInfo = "Failed to load file : " + errorInfo;
			MessageBoxA( NULL , errorInfo.c_str(), "Failed to loading files in zip", MB_ICONERROR | MB_OK );
			*success = false;
		}

		*bufferLength = ze.unc_size + 1;
		unsigned char* xmlBuffer = new unsigned char[ze.unc_size + 1];
		xmlBuffer[ze.unc_size] = '\0';
		UnzipItem( zipFile , index , xmlBuffer , ze.unc_size );

		return xmlBuffer;
	}
	else
	{
		LoadZipToMap(zipFilePath, password);
		HZIP newZipFile = (HZIP)GetZip(zipFilePath);
		if(newZipFile)
		{
			ZIPENTRY ze;
			int index;
			USES_CONVERSION;
			TCHAR* lpString = pathInZip.substr(0,2) == "./" ? A2T(pathInZip.substr(2).c_str()) : A2T(pathInZip.c_str());
			FindZipItem(newZipFile, lpString, true, &index, &ze);
			if(index == -1)
			{
				std::string errorInfo(pathInZip);
				errorInfo = "Failed to load file : " + errorInfo;
				MessageBoxA( NULL , errorInfo.c_str(), "Failed to loading files in zip", MB_ICONERROR | MB_OK );
				*success = false;
			}

			unsigned char* xmlBuffer = new unsigned char[ze.unc_size + 1];
			xmlBuffer[ze.unc_size] = '\0';
			UnzipItem( newZipFile , index , xmlBuffer , ze.unc_size );
			
			return xmlBuffer;
		}
		else
		{
			//assertion
			std::string errorInfo(pathInZip);
			errorInfo = "Failed to load file : " + errorInfo;
			MessageBoxA( NULL , errorInfo.c_str(), "Failed to loading files in zip", MB_ICONERROR | MB_OK );

			*success = false;
			exit(0);
		}
	}
}


void* ZipHelper::GetZip(const std::string& zipFilePath)
{
	std::string targetName = zipFilePath;
	std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::tolower);

	std::map< std::string , void* >::iterator it = s_zipMap.begin();
	while(it != s_zipMap.end())
	{
		if(it->first == targetName)
		{
			return it->second;
		}
		++it;
	}

	return nullptr;
}

};