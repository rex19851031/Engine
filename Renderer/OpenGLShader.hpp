#pragma once

#ifndef OPENGLSHADER_HPP
#define OPENGLSHADER_HPP

#include "Engine\Renderer\OpenGLRenderer.hpp"


namespace Henry
{

class OpenGLShader
{
public:
	OpenGLShader(char* shaderFilePath , GLenum shader_type , bool* success = nullptr);
	OpenGLShader(char* shaderFilePath , GLenum shader_type , const std::string& zipFile , bool* success);
	~OpenGLShader(void);
	int LoadFileToNewBuffer(char* shaderFilePath,char*& shaderTextBuffer,bool dunno = true);
	void ReportShaderError(GLenum m_shaderType, int m_shaderID, const char* shaderText, char* m_shaderFileName, char* m_shaderFilePath );

	int m_shaderID;
	GLenum m_shaderType;
private:
	void GetFileNameFromPath(char* filePath);
	void AnalysisError(std::string& errorLog , std::string& result);
	void GetErrorMessageFromLog(std::string errorLog,std::string& lineNum,std::string& errorCode,std::string& errorDetail);
	std::string intToString(int number);
	char* m_shaderFilePath;
	char* m_shaderFileName;
};

};

#endif