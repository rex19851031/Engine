#pragma once

#ifndef BUFFERPARSER_HPP
#define BUFFERPARSER_HPP

#include "Engine\Core\VertexStruct.hpp"

#include <string>
#include <vector>

namespace Henry
{

class BufferParser
{
public:
	BufferParser();
	BufferParser(const char* filePath);
	~BufferParser(void);
	bool LoadFile(const char* filePath);
	bool WriteFile(const char* filePath,const std::vector<Vertex_PCTN>& verticesToWrite,const std::vector<int>& indicesToWrite);
	void Rewind();
	bool ReadInt(int& out_int);
	bool ReadUInt(unsigned int& out_int);
	bool ReadFloat(float& out_float);
	bool ReadBool(bool& out_bool);
	bool ReadString(std::string& out_string);
	bool ReadChar(char& out_char);
	bool ReadUChar(unsigned char& our_char);

	const unsigned char* m_scan;
	unsigned const char* m_end;
private:
	bool WriteFloat(std::vector<unsigned char>& buffer,const float data);
	bool WriteChar(std::vector<unsigned char>& buffer,const char data);
	bool WriteInt(std::vector<unsigned char>& buffer,const int data);
	bool WriteUInt(std::vector<unsigned char>& buffer,const unsigned int data);
	bool WriteBool(std::vector<unsigned char>& buffer,const bool data);
	bool WriteString(std::vector<unsigned char>& buffer,const std::string& data);
	bool WriteUChar(std::vector<unsigned char>& buffer,const unsigned char data);
	unsigned char* m_buffer;
	int m_fileSize;
	//int m_writeIndex;
};

};

#endif