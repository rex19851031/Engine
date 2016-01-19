#include "FileSystem.hpp"

#include <io.h>
#include <windows.h>
#include <sys/stat.h>
#include <time.h>

namespace Henry
{

FileSystem::FileSystem(void)
{
}


FileSystem::~FileSystem(void)
{
}


void FileSystem::EnumerateFileInFolder( const char* directory , std::vector<std::string>& out_fileNames )
{
	int	error = 0;
	struct _finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( directory , &fileInfo );

	static int recursiveTimes = 0;
	recursiveTimes++;

	while(searchHandle != -1 && !error)
	{
		bool isADirectory = fileInfo.attrib & _A_SUBDIR ? true : false;
		bool isHidden = fileInfo.attrib & _A_HIDDEN ? true : false;
		if(!isADirectory && !isHidden)
		{
			out_fileNames.push_back(fileInfo.name);
		}
		error = _findnext(searchHandle,&fileInfo);
	}
}


void FileSystem::EnumerateFilesInFolder( const char* directory , std::vector<std::string>& out_fullFilePaths , std::vector<std::string>& out_filenames , bool isRecursive )
{
	// improve : the file under root directory , push without directory
	std::string fullSearchCmd(directory);
	fullSearchCmd.append("/*.*");
	
	int	error = 0;
	struct _finddata_t fileInfo;
	intptr_t searchHandle = _findfirst(fullSearchCmd.c_str(),&fileInfo);

	static int recursiveTimes = 0;
	recursiveTimes++;

	while(searchHandle != -1 && !error)
	{
		bool isADirectory = fileInfo.attrib & _A_SUBDIR ? true : false;
		bool isHidden = fileInfo.attrib & _A_HIDDEN ? true : false;
		if(isRecursive && isADirectory && strcmp(fileInfo.name,".") && strcmp(fileInfo.name,".."))
		{
			printf("find subDirectory : %s\n",fileInfo.name);
			std::string subPath(directory);
			subPath.append("/");
			subPath.append(fileInfo.name);
			EnumerateFilesInFolder(subPath.c_str() , out_fullFilePaths , out_filenames , isRecursive );
		}
		else if(!isADirectory && !isHidden)
		{
			std::string subDirectory(directory);
			subDirectory.append("/");
			subDirectory.append(fileInfo.name);
			out_fullFilePaths.push_back(subDirectory);
			out_filenames.push_back(fileInfo.name);
		}
		error = _findnext(searchHandle,&fileInfo);
	}
}


bool FileSystem::CopyTheFile( const char* sourceFile , const char* newFileName , const char* destinationFolder , bool overwrite)
{
	CreateFolder(destinationFolder);

	std::string outPath(destinationFolder);
	outPath.append(newFileName);

	LPCWSTR source = convertCharArrayToLPCWSTR(sourceFile);
	LPCWSTR destination = convertCharArrayToLPCWSTR(outPath.c_str());

	bool result = CopyFile( source , destination , !overwrite ) == TRUE ? true : false;
	return result;
}


wchar_t* FileSystem::convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}


void FileSystem::GetExtension( std::string& fileName , std::string& out_result )
{
	size_t count = 1;
	for(size_t index = fileName.size()-1; index > 0; --index)
	{
		if(fileName[index] != '.')
			count++;
		else
			break;
	}
	if(count <= fileName.size())
		out_result = fileName.substr(fileName.size() - count , count);
}


void FileSystem::RemoveExtension( std::string& fileName , std::string& out_result )
{
	size_t count = 1;
	for(size_t index = fileName.size()-1; index > 0; --index)
	{
		if(fileName[index] != '.')
			count++;
		else
			break;
	}

	if(count <= fileName.size())
		out_result = fileName.substr( 0 , fileName.size() - count );
}


void FileSystem::CreateFolder( const char* folderPath )
{
	struct stat info;
	if( stat( folderPath , &info ) != 0 )
	{
		LPCWSTR outFolder = convertCharArrayToLPCWSTR(folderPath);
		CreateDirectory(outFolder,NULL);
	}
}


bool FileSystem::CompareTime( const char* olderFile , const char* newerFile )
{
	struct stat attrib_older;
	struct stat attrib_newer;
	struct stat info;

	if( stat( olderFile , &info ) != 0 || stat( newerFile , &info ) != 0 )
		return true;

	stat(olderFile, &attrib_older);
	stat(newerFile, &attrib_newer);
	
	// 	printf(" older : %s , newer : %s \n",olderFile , newerFile );
	// 	printf(" older : %lf , newer : %lf \n",mktime(clock_older) , mktime(clock_newer)  );
	return attrib_older.st_mtime <= attrib_newer.st_mtime ? true : false;
}

};