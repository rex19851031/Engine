#include <sstream>
#include <string>
#include <Windows.h>

#include "ObjLoader.hpp"

namespace Henry
{

ObjLoader::ObjLoader(void)
{
	m_xBasis = Vec3f( 1.0f , 0.0f , 0.0f );
	m_yBasis = Vec3f( 0.0f , 1.0f , 0.0f );
	m_zBasis = Vec3f( 0.0f , 0.0f , 1.0f );
	m_xScaledBasis = m_xBasis;
	m_yScaledBasis = m_yBasis;
	m_zScaledBasis = m_zBasis;
	m_scale = 1.0f;
}


ObjLoader::~ObjLoader(void)
{

}


bool ObjLoader::LoadFile(const char* path)
{
	FILE *file;
	fopen_s(&file,path,"rb");

	if(file == NULL)
	{
		std::string errorInfo("Can't find the file : \n");
		errorInfo.append(path);
		errorInfo.append("\n\nMake sure the file is under the correct folder.");
		errorInfo.append("\n\nThe Program Will Close Automatically.");
		MessageBoxA( NULL , errorInfo.c_str(), "Failed to loading files", MB_ICONERROR | MB_OK );
		exit(0);
	}

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = new char[fsize];
	fread(buffer, sizeof(char), fsize, file);
	fclose(file);

	ParseData(buffer,fsize);
	delete[] buffer;

	return true;
}


void ObjLoader::ParseData(const char* buffer,int dataSize)
{
	int dataIndex = 0;
	enum dataType{ none , vertexData , metaData , textureData , normalData , face};
	dataType type = none;

	while(dataIndex < dataSize)
	{
		char data = (char)buffer[++dataIndex];

		if(IsLineEnded(buffer,dataIndex-1) && data == 'v')
		{
			dataIndex++;

			if(buffer[dataIndex] == ' ' )
				type = vertexData;
			if(buffer[dataIndex] == 'n' && buffer[++dataIndex] == ' ')
				type = normalData;
			if(buffer[dataIndex] == 't' && buffer[++dataIndex] == ' ')
				type = textureData;

			SeekToNonWhiteSpace(buffer,&dataIndex);
		}

		if(IsLineEnded(buffer,dataIndex-1) && data == 'f' && buffer[++dataIndex] == ' ')
		{
			SeekToNonWhiteSpace(buffer,&dataIndex);
			type = face;
		}

		if(IsLineEnded(buffer,dataIndex-1) && data == '#' && buffer[++dataIndex] == ' ')
		{
			char temp[256];
			char* result = temp;
			memset(temp, 0, 256);

			SeekToNonWhiteSpace(buffer,&dataIndex);
			CopyUntilWhitespaceOrEOL(buffer, dataIndex, result);
			
			char toLower[256];
			memset(toLower,0,256);
			ToLower(result,toLower);

			std::string strForCompare(toLower);
			
			if(strcmp(toLower,"metadata") == 0)
				type = metaData;
			
			SeekToNonWhiteSpace(buffer,&dataIndex);
		}

		if(type == metaData)
		{
			ParsingMetaData(buffer,&dataIndex);
		}

		if(type == vertexData)
		{
			ParsingVertexData(buffer,&dataIndex);
		}

		if(type == normalData)
		{
			ParsingNormalData(buffer,&dataIndex);
		}

		if(type == textureData)
		{
			ParsingTextureData(buffer,&dataIndex);
		}

		if(type == face)
		{
			ParsingFaceData(buffer,&dataIndex);
		}

		type = none;
	}
}


void ObjLoader::SeekToNonWhiteSpace(const char* buffer, int* dataIndex)
{
	int& index = *dataIndex;
	while(buffer[index] == ' ' || buffer[index] == '\t' || IsLineEnded(buffer,index))
		index++;
}


void ObjLoader::CopyUntilWhitespaceOrEOL(const char* buffer, int& dataIndex, char*& result)
{
	while(buffer[dataIndex] != ' ' && buffer[dataIndex] != '\t' && !IsLineEnded(buffer, dataIndex))
// 	{
// 		*result = buffer[ dataIndex ];
// 		++ result;
// 		++ dataIndex;
// 	}
		strncat_s(result, 256, &buffer[dataIndex++], 1);//strncat(result, &buffer[index++], 1);
}


bool ObjLoader::IsLineEnded(const char* buffer,int dataIndex)
{
	return (buffer[dataIndex] == '\n') || (buffer[dataIndex] == '\r') || (buffer[dataIndex]=='\0');
}


void ObjLoader::ParsingMetaData(const char* buffer,int* dataIndex)
{
	enum orientationDataSection {empty,xPart,yPart,zPart};
	orientationDataSection ori_section = empty;

	enum dataSection {none,orientationSection,scaleSection};
	bool captureEqualSymbol = false;
	dataSection section = none;
	int& index = *dataIndex;

	while(!IsLineEnded(buffer,index))
	{
		char data = (char)tolower(buffer[index]);
		switch(data)
		{
		case 'x':
			ori_section = xPart;
			break;
		case 'y':
			ori_section = yPart;
			break;
		case 'z':
			ori_section = zPart;
			break;
		}

		if(data != ' ' && section == none)
		{
			char type[256];
			memset(type, 0, 256);
			char* result = type;
			CopyUntilWhitespaceOrEOL(buffer, index, result);

			char toLower[256];
			memset(toLower, 0, 256);
			ToLower(type,toLower);

			if(strcmp(toLower,"orientation") == 0)
				section = orientationSection;
			if(strcmp(toLower,"scale") == 0)
				section = scaleSection;
		}

		if(section != none && data == '=')
			captureEqualSymbol = true;

		if(data != '=' && captureEqualSymbol && data != ' ')
		{
			char value[256];
			memset(value, 0, 256);
			char* result = value;
			CopyUntilWhitespaceOrEOL(buffer, index, result);

			if(section == orientationSection)
			{
				if(ori_section == xPart)
					m_xBasis = GetAxisBasis(value);
				if(ori_section == yPart)
					m_yBasis = GetAxisBasis(value);
				if(ori_section == zPart)
					m_zBasis = GetAxisBasis(value);
			}

			if(section == scaleSection)
			{
				SetScale(value);
			}

			captureEqualSymbol = false;
		}

		if(IsLineEnded(buffer,index))
			break;

		index++;
	}
}


Vec3f ObjLoader::GetAxisBasis(const char* direction)
{
	char toLowerDirection[256];
	memset(&toLowerDirection[0], 0, 256);
	ToLower(direction,toLowerDirection);

	if(strcmp(toLowerDirection,"up") == 0 || strcmp(toLowerDirection,"top") == 0)
		return Vec3f(0,0,1);

	if(strcmp(toLowerDirection,"down") == 0 || strcmp(toLowerDirection,"bottom") == 0)
		return Vec3f(0,0,-1);

	if(strcmp(toLowerDirection,"left") == 0 || strcmp(toLowerDirection,"north") == 0)
		return Vec3f(0,1,0);

	if(strcmp(toLowerDirection,"right") == 0 || strcmp(toLowerDirection,"south") == 0)
		return Vec3f(0,-1,0);

	if(strcmp(toLowerDirection,"forward") == 0 || strcmp(toLowerDirection,"east") == 0)
		return Vec3f(1,0,0);

	if(strcmp(toLowerDirection,"backward") == 0 || strcmp(toLowerDirection,"west") == 0)
		return Vec3f(-1,0,0);

	return Vec3f(0,0,0);
}


Vec3f ObjLoader::ChangeOfBasis(Vec3f vertex,bool doScale)
{
	Vec3f result;
	if(doScale)
	{
		
		result.x = m_xScaledBasis.x * vertex.x + m_yScaledBasis.x * vertex.y + m_zScaledBasis.x * vertex.z;
		result.y = m_xScaledBasis.y * vertex.x + m_yScaledBasis.y * vertex.y + m_zScaledBasis.y * vertex.z;
		result.z = m_xScaledBasis.z * vertex.x + m_yScaledBasis.z * vertex.y + m_zScaledBasis.z * vertex.z;
	}
	else
	{
		result.x = m_xBasis.x * vertex.x + m_yBasis.x * vertex.y + m_zBasis.x * vertex.z;
		result.y = m_xBasis.y * vertex.x + m_yBasis.y * vertex.y + m_zBasis.y * vertex.z;
		result.z = m_xBasis.z * vertex.x + m_yBasis.z * vertex.y + m_zBasis.z * vertex.z;
	}
	return result;
}


void ObjLoader::SetScale(const char* scale)
{
	m_scale = (float)(1.0f/atof(scale));
	m_xScaledBasis = m_xBasis * m_scale;
	m_yScaledBasis = m_yBasis * m_scale;
	m_zScaledBasis = m_zBasis * m_scale;
}


void ObjLoader::ToLower(const char* source, char* dst)
{
	int index = 0;
	while(source[index])
		dst[index] = source[index++];
}


void ObjLoader::ParsingTextureData(const char* buffer,int* dataIndex)
{
	int& index = *dataIndex;
	int variableIndex = 0;
	Vec3f textCoord;
	char temp[256];

	while(!IsLineEnded(buffer,index))
	{
		char* result = temp;
		memset(temp,0,256);
		CopyUntilWhitespaceOrEOL(buffer, index, result);

		switch(variableIndex)
		{
		case 0:
			textCoord.x = (float)std::atof(temp);
			break;
		case 1:
			textCoord.y = (float)std::atof(temp);
			break;
		case 2:
			textCoord.z = (float)std::atof(temp);
			break;
		}

		if(IsLineEnded(buffer,index))
			break;

		variableIndex++;
		index++;
	}

	m_rawTextureList.push_back(textCoord);
}


void ObjLoader::ParsingVertexData(const char* buffer,int* dataIndex)
{
	int& index = *dataIndex;
	int variableIndex = 0;
	Vec3f vertex;
	RGBA color;
	char temp[256];
	float w = 1;

	while(!IsLineEnded(buffer,index))
	{
		char* result = temp;
		memset(temp,0,256);
		CopyUntilWhitespaceOrEOL(buffer, index, result);

		switch(variableIndex)
		{
		case 0:
			vertex.x = (float)std::atof(temp);
			break;
		case 1:
			vertex.y = (float)std::atof(temp);
			break;
		case 2:
			vertex.z = (float)std::atof(temp);
			break;
		case 3:
			color.r = (unsigned char)(std::atof(temp) * 255);
			break;
		case 4:
			color.g = (unsigned char)(std::atof(temp) * 255);
			break;
		case 5:
			color.b = (unsigned char)(std::atof(temp) * 255);
			break;
		case 6:
			w = color.r;
			color.r = color.g;
			color.g = color.b;
			color.b = (unsigned char)(std::atof(temp) * 255);
			break;
		}

		if( IsLineEnded( buffer, index ) )
			break;

		variableIndex++;
		index++;
	}

	vertex = ChangeOfBasis(vertex,true);
	m_rawVertexList.push_back(vertex);
	m_rawWList.push_back(w);
	m_rawColorList.push_back(color);
}


void ObjLoader::ParsingNormalData(const char* buffer,int* dataIndex)
{
	int& index = *dataIndex;
	int variableIndex = 0;
	Vec3f normal;
	char temp[256];

	while(!IsLineEnded(buffer,index))
	{
		char* result = temp;
		memset(temp,0,256);
		CopyUntilWhitespaceOrEOL(buffer, index, result);

		switch(variableIndex)
		{
		case 0:
			normal.x = (float)std::atof(temp);
			break;
		case 1:
			normal.y = (float)std::atof(temp);
			break;
		case 2:
			normal.z = (float)std::atof(temp);
			break;
		}

		if(IsLineEnded(buffer,index))
			break;

		variableIndex++;
		index++;
	}

	normal = ChangeOfBasis(normal,false);
	m_rawNormalList.push_back(normal);
}


void ObjLoader::ParsingFaceData(const char* buffer,int* dataIndex)
{
	int& index = *dataIndex;
	std::vector<int> vertexIndicesList;
	std::vector<int> textureIndicesList;
	std::vector<int> normalIndicesList;
	char vertexAsText[256];
	char vertexComponentAsText[256];
	int vertexComponentAsInt;
	vertexIndicesList.reserve(6);
	normalIndicesList.reserve(6);
	textureIndicesList.reserve(6);
	if(CheckTokenInLine(buffer,index,'/'))
	{
		while(!IsLineEnded(buffer,index))
		{
			char* writeLocation = vertexAsText;
			memset(vertexAsText,0,256);
			CopyUntilWhitespaceOrEOL(buffer, index, writeLocation);

			writeLocation = vertexComponentAsText;
			memset(vertexComponentAsText,0,256);
			int currentIndex = 0;
			SeekToToken(vertexAsText,&currentIndex,'/',writeLocation);
			vertexComponentAsInt = std::atoi(vertexComponentAsText);
			if(vertexComponentAsInt < 0)
				vertexComponentAsInt += m_rawVertexList.size() + 1;
			if(vertexComponentAsInt == 0)
				vertexComponentAsInt++;
			vertexIndicesList.push_back(vertexComponentAsInt);
			currentIndex++;

			writeLocation = vertexComponentAsText;
			memset(vertexComponentAsText,0,256);
			SeekToToken(vertexAsText,&currentIndex,'/',writeLocation);
			vertexComponentAsInt = std::atoi(vertexComponentAsText);
			if(vertexComponentAsInt < 0)
				vertexComponentAsInt += m_rawTextureList.size() + 1;
			if(vertexComponentAsInt == 0)
				vertexComponentAsInt++;
			textureIndicesList.push_back(vertexComponentAsInt);
			currentIndex++;

			writeLocation = vertexComponentAsText;
			memset(vertexComponentAsText,0,256);
			CopyUntilWhitespaceOrEOL(vertexAsText, currentIndex, writeLocation);
			vertexComponentAsInt = std::atoi(vertexComponentAsText);
			if(vertexComponentAsInt < 0)
				vertexComponentAsInt += m_rawNormalList.size() + 1;
			if(vertexComponentAsInt == 0)
				vertexComponentAsInt++;
			normalIndicesList.push_back(vertexComponentAsInt);

			if(IsLineEnded(buffer,index))
				break;

			index++;
		}
	}
	else
	{
		Vec3f normal;
		char temp[256];

		while(!IsLineEnded(buffer,index))
		{
			char* result = temp;
			memset(temp,0,256);
			CopyUntilWhitespaceOrEOL(buffer, index, result);

			vertexIndicesList.push_back(std::atoi(temp));

			if(IsLineEnded(buffer,index))
				break;

			index++;
		}
	}

	for(size_t startIndex = 1; startIndex + 1 < vertexIndicesList.size(); startIndex++)
	{
		m_vertexIndexList.push_back(vertexIndicesList[0]);
		m_vertexIndexList.push_back(vertexIndicesList[startIndex]);
		m_vertexIndexList.push_back(vertexIndicesList[startIndex+1]);
		
		if(textureIndicesList.size() != 0)
		{
			m_textureIndexList.push_back(textureIndicesList[0]);
			m_textureIndexList.push_back(textureIndicesList[startIndex]);
			m_textureIndexList.push_back(textureIndicesList[startIndex+1]);
		}

		if(normalIndicesList.size() != 0)
		{
			m_normalIndexList.push_back(normalIndicesList[0]);
			m_normalIndexList.push_back(normalIndicesList[startIndex]);
			m_normalIndexList.push_back(normalIndicesList[startIndex+1]);
		}
	}
}


void ObjLoader::SeekToToken(const char* buffer,int* dataIndex, const char token,char*& result)
{
	int& index = *dataIndex;
	while(buffer[index] != token && buffer[index] > 0 && !IsLineEnded(buffer,index))
		strncat_s(result, 256, &buffer[index++], 1);
}


void ObjLoader::Empty()
{
	std::vector<Vec3f> vec3_empty;
	std::vector<int> int_empty;
	std::vector<float> float_empty;
	std::vector<RGBA> color_empty;

	m_rawVertexList = vec3_empty;
	m_rawNormalList = vec3_empty;
	m_rawTextureList = vec3_empty;
	m_textureIndexList = int_empty;
	m_normalIndexList = int_empty;
	m_vertexIndexList = int_empty;
	m_rawColorList = color_empty;
	m_rawWList = float_empty;

	m_xBasis = Vec3f( 1.0f , 0.0f , 0.0f );
	m_yBasis = Vec3f( 0.0f , 1.0f , 0.0f );
	m_zBasis = Vec3f( 0.0f , 0.0f , 1.0f );
	m_xScaledBasis = m_xBasis;
	m_yScaledBasis = m_yBasis;
	m_zScaledBasis = m_zBasis;
	m_scale = 1.0f;
}


bool ObjLoader::CheckTokenInLine(const char* buffer,int dataIndex,const char token)
{
	bool containToken = false;
	while(!IsLineEnded(buffer,dataIndex))
	{
		if(buffer[dataIndex] == token)
			containToken = true;

		if(IsLineEnded(buffer,dataIndex))
			break;

		dataIndex++;
	}

	return containToken;
}

};