#include "OpenGLShader.hpp"

#include "Engine\Renderer\glext.h"
#include "Engine\Parsing\ZipUtils\ZipHelper.hpp"

#include <sstream>


namespace Henry
{

#define UNUSED(x) (void)(x);

extern PFNGLCREATESHADERPROC		glCreateShader;
extern PFNGLSHADERSOURCEPROC		glShaderSource;
extern PFNGLCREATESHADERPROC		glCreateShader;		
extern PFNGLSHADERSOURCEPROC		glShaderSource;		
extern PFNGLCOMPILESHADERPROC		glCompileShader;		
extern PFNGLGETSHADERIVPROC			glGetShaderiv;		
extern PFNGLGETSHADERINFOLOGPROC	glGetShaderInfoLog;			

OpenGLShader::OpenGLShader(char* shader_file_path , GLenum shader_type , bool* success) 
	: m_shaderFilePath (shader_file_path) 
	, m_shaderType(shader_type)
{
	if( success != nullptr)
		*success = true;
	char* shaderTextBuffer;
	GLint wasSuccessful;

	GetFileNameFromPath( m_shaderFilePath );
	LoadFileToNewBuffer(shader_file_path , shaderTextBuffer);
	m_shaderID = glCreateShader(shader_type);
	glShaderSource( m_shaderID, 1, &shaderTextBuffer, nullptr );
	glCompileShader( m_shaderID );
	glGetShaderiv( m_shaderID, GL_COMPILE_STATUS, &wasSuccessful );
	
	if( !wasSuccessful )
	{
		ReportShaderError( m_shaderType , m_shaderID , shaderTextBuffer , m_shaderFileName , m_shaderFilePath );
		if(success != nullptr)
			*success = false;
	}
	
	delete[] shaderTextBuffer;
}


OpenGLShader::OpenGLShader(char* shaderFilePath , GLenum shader_type ,  const std::string& zipFile , bool* success)
	: m_shaderFilePath (shaderFilePath) 
	, m_shaderType(shader_type)
{
	*success = true;
	GLint wasSuccessful;
	int len;
	const unsigned char* shaderTextBuffer = ZipHelper::GetContentInZip(zipFile,shaderFilePath,"c23",&len,success);
	m_shaderID = glCreateShader(shader_type);
	glShaderSource( m_shaderID, 1, (const GLchar* const*)&shaderTextBuffer, nullptr );
	glCompileShader( m_shaderID );
	glGetShaderiv( m_shaderID, GL_COMPILE_STATUS, &wasSuccessful );

	if( !wasSuccessful )
	{
		ReportShaderError( m_shaderType , m_shaderID , (const char*)shaderTextBuffer , m_shaderFileName , m_shaderFilePath );
		*success = false;
	}

	delete[] shaderTextBuffer;
}


OpenGLShader::~OpenGLShader(void)
{
	delete[] m_shaderFilePath;
	delete[] m_shaderFileName;
}


int OpenGLShader::LoadFileToNewBuffer(char* shaderFilePath,char*& shaderTextBuffer,bool dunno)
{
	UNUSED(dunno);
	FILE *file;
	fopen_s( &file , shaderFilePath , "rb" );
	if(file == NULL)
	{
		std::string errorInfo(shaderFilePath);
		errorInfo = "Failed to load file : " + errorInfo;
		MessageBoxA( NULL , errorInfo.c_str(), "Failed to loading files", MB_ICONERROR | MB_OK );
		exit(0);
	}

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	int bytesRead = fsize;
	shaderTextBuffer = new char[bytesRead + 1];
	fread(shaderTextBuffer, sizeof(char), bytesRead, file);
	fclose(file);
	shaderTextBuffer[bytesRead] = '\0';

	return bytesRead;
}


void OpenGLShader::ReportShaderError(GLenum m_shaderType, int m_shaderID,const char* shaderText, char* m_shaderFileName, char* m_shaderFilePath )
{
	UNUSED(m_shaderFilePath);
	UNUSED(m_shaderType);
	UNUSED(shaderText);

	std::string errorMessage;

	GLint infoLogLength;
	glGetShaderiv( m_shaderID, GL_INFO_LOG_LENGTH, &infoLogLength );
	char* infoLogText = new char[ infoLogLength + 1 ];
	glGetShaderInfoLog( m_shaderID, infoLogLength, NULL, infoLogText );
	std::string infoLogString( infoLogText );
	delete[] infoLogText;
	AnalysisError(infoLogString,errorMessage);
	infoLogString = errorMessage;
	std::string errorTitle( m_shaderFileName );
	errorTitle = "SD3 Demo :: GLSL compiler error in " + errorTitle;
	MessageBoxA( NULL , (LPCSTR)infoLogString.c_str() , (LPCSTR)errorTitle.c_str(), MB_ICONERROR | MB_OK );
}


void OpenGLShader::GetFileNameFromPath(char* c_filePath)
{
	std::string filePath(c_filePath);
	std::string fileName;

	for(size_t index=0; index < filePath.length(); index++)
	{
		if(filePath[index] == '\\' || filePath[index] == '/' )
		{
			fileName = "";
		}
		else
		{
			fileName += filePath[index];
		}
	}

	m_shaderFileName = new char[fileName.length()];
	strcpy(m_shaderFileName,fileName.c_str());
}


void OpenGLShader::AnalysisError(std::string& errorLog , std::string& result)
{
	std::string lineNum;
	std::string errorCode;
	std::string errorDetail;
	char fullPath[_MAX_PATH];
	_fullpath( fullPath, m_shaderFilePath, _MAX_PATH );
	std::string filePath(fullPath);

	GetErrorMessageFromLog(errorLog,lineNum,errorCode,errorDetail);
	if(m_shaderType == OpenGLRenderer::VERTEX_SHADER)
		result = "GLSL vertex shader compiler error!\n\n";
	if(m_shaderType == OpenGLRenderer::FRAGMENT_SHADER)
		result = "GLSL fragment shader compiler error!\n\n";
	result += m_shaderFileName;
	result += " , line : " + lineNum + "\n\n";
	result += "ERROR : " + errorDetail + "\n";
	result += "Current OpenGL version : " + OpenGLRenderer::GetString(OpenGLRenderer::VERSION) + "\n\n";
	result += "Current GLSL version : " + OpenGLRenderer::GetString(OpenGLRenderer::SHADING_LANGUAGE_VERSION) + "\n\n";
	result += "File Location : " ;
	result += fullPath;
	result += "\n\nRaw Error Log :\n" + errorLog + "\n";
	result += "This application will now closed.";

	OutputDebugStringA( (filePath + "(" + lineNum + "):" + errorCode + " :" + errorDetail.substr(0,errorDetail.size()-1) + " : " + " OpenGL version -> " + OpenGLRenderer::GetString(OpenGLRenderer::VERSION) + " : GLSL version -> " + OpenGLRenderer::GetString(OpenGLRenderer::SHADING_LANGUAGE_VERSION) + "\n").c_str() );
}


void OpenGLShader::GetErrorMessageFromLog(std::string errorLog,std::string& lineNum,std::string& errorCode,std::string& errorDetail)
{
	int num = 0;
	bool codeZone = false;
	bool detailZone = false;
	for(size_t index=2; index < errorLog.length(); index++)
	{
		if( errorLog[index] != ')' && lineNum.length() == 0)
			num = num * 10 + (int)errorLog[index] - (int)'0';
		else
			lineNum = intToString(num);

		if(detailZone)
			errorDetail += errorLog[index];

		if(codeZone)
		{
			if(errorLog[index] != ':')
				errorCode += errorLog[index];
			else
			{
				codeZone = false;
				detailZone = true;
			}
		}

		if( errorLog[index] == ':' && errorCode.length() == 0)
			codeZone = true;
	}
}


std::string OpenGLShader::intToString(int number)
{
	std::stringstream ss;
	ss << number;
	return ss.str();
}

};