#pragma once

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <vector>

namespace Henry
{

class FileSystem
{
public:
	FileSystem(void);
	~FileSystem(void);
	static void GetExtension( std::string& fileName , std::string& out_result );
	static void RemoveExtension( std::string& fileName , std::string& out_result );
	bool CopyTheFile( const char* sourceFile , const char* newFileName , const char* destinationFolder , bool overwritre = false);
	void CreateFolder( const char* folderPath );
	void EnumerateFilesInFolder( const char* directory , std::vector<std::string>& out_fullFilePaths  , std::vector<std::string>& out_filenames , bool isRecursive = false);
	void EnumerateFileInFolder( const char* directory , std::vector<std::string>& out_fileNames );
	bool CompareTime( const char* olderFile , const char* newerFile );
private:
	wchar_t* convertCharArrayToLPCWSTR(const char* charArray);
};

};

#endif