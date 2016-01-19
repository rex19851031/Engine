#pragma once

#ifndef OBJLOADER_HPP 
#define OBJLOADER_HPP

#include <vector>

#include "Engine\Math\Vec3.hpp"
#include "Engine\Core\VertexStruct.hpp"

namespace Henry
{

class ObjLoader
{
public:
	ObjLoader(void);
	~ObjLoader(void);
	bool LoadFile(const char* path);
	void Empty();
	std::vector<Vec3f> m_rawVertexList;
	std::vector<Vec3f> m_rawNormalList;
	std::vector<Vec3f> m_rawTextureList;
	std::vector<RGBA> m_rawColorList;
	std::vector<float> m_rawWList;
	std::vector<int> m_normalIndexList;
	std::vector<int> m_vertexIndexList;
	std::vector<int> m_textureIndexList;
private:
	void SeekToNonWhiteSpace(const char* buffer,int* dataIndex);
	void CopyUntilWhitespaceOrEOL(const char* buffer, int& dataIndex,char*& result);
	void SeekToToken(const char* buffer,int* dataIndex, const char token,char*& result);
	void ScanForToken(const char* scan);
	bool IsLineEnded(const char* buffer,int dataIndex);
	void ParseData(const char* buffer,int dataSize);
	void ParsingMetaData(const char* buffer,int* dataIndex);
	void ParsingVertexData(const char* buffer,int* dataIndex);
	void ParsingNormalData(const char* buffer,int* dataIndex);
	void ParsingTextureData(const char* buffer,int* dataIndex);
	void ParsingFaceData(const char* buffer,int* dataIndex);
	bool CheckTokenInLine(const char* buffer,int dataIndex,const char token);
	Vec3f ChangeOfBasis(Vec3f vertex,bool doScale);
	Vec3f GetAxisBasis(const char* direction);
	void SetScale(const char* scale);
	void ToLower(const char* source, char* dst);
	Vec3f m_xBasis;
	Vec3f m_yBasis;
	Vec3f m_zBasis;
	Vec3f m_xScaledBasis;
	Vec3f m_yScaledBasis;
	Vec3f m_zScaledBasis;
	float m_scale;
};

};

#endif