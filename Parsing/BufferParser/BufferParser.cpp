#include "BufferParser.hpp"

#include <Windows.h>
#include <sstream>

namespace Henry
{


BufferParser::BufferParser() : m_fileSize(0) , m_buffer(nullptr) , m_scan(nullptr) , m_end(nullptr)
{

}


BufferParser::BufferParser(const char* filePath) : m_fileSize(0) , m_buffer(nullptr) , m_scan(nullptr) , m_end(nullptr)
{
	LoadFile(filePath);
}


BufferParser::~BufferParser(void)
{
	if(m_buffer)
		delete[] m_buffer;
	if(m_scan)
		delete m_scan;
	if(m_end)
		delete m_end;
}


bool BufferParser::LoadFile(const char* filePath)
{
	if(m_fileSize != 0)
		delete[] m_buffer;

	FILE *file;
	fopen_s(&file,filePath,"rb");

	if(file == NULL)
	{
		std::string errorInfo("Can't find the file : \n\t");
		errorInfo.append(filePath);
		errorInfo.append("\n\nMake sure the file already been cooked.");
		errorInfo.append("\n\nThe Program Will Close Automatically.");
		MessageBoxA( NULL , errorInfo.c_str(), "Failed to loading files", MB_ICONERROR | MB_OK );
		exit(0);
	}

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	m_buffer = new unsigned char[fsize];
	fread(m_buffer, sizeof(unsigned char), fsize, file);
	fclose(file);

	m_scan = m_buffer;
	m_end = m_buffer + fsize * sizeof(unsigned char);
	return true;
}


bool BufferParser::WriteFile(const char* filePath,const std::vector<Vertex_PCTN>& verticesToWrite,const std::vector<int>& indicesToWrite)
{
	std::vector<unsigned char> writeBuffer;
	writeBuffer.reserve(verticesToWrite.size() * sizeof(Vertex_PCTN) + 1024);
	
	WriteChar(writeBuffer,'G');
	WriteChar(writeBuffer,'C');
	WriteChar(writeBuffer,'2');
	WriteChar(writeBuffer,'3');
	WriteChar(writeBuffer,2);
	WriteChar(writeBuffer,1);
	WriteString(writeBuffer,"test file 1");
	WriteUInt(writeBuffer,(unsigned int)indicesToWrite.size());
	
	for(size_t index=0; index<indicesToWrite.size(); index++)
	{
		WriteUInt(writeBuffer,indicesToWrite[index]);
	}
	
	WriteUInt(writeBuffer,(unsigned int)verticesToWrite.size());
	
	for(size_t index=0; index<verticesToWrite.size(); index++)
	{
		WriteFloat(writeBuffer,verticesToWrite[index].position.x);
		WriteFloat(writeBuffer,verticesToWrite[index].position.y);
		WriteFloat(writeBuffer,verticesToWrite[index].position.z);

		WriteUChar(writeBuffer,verticesToWrite[index].color.r);
		WriteUChar(writeBuffer,verticesToWrite[index].color.g);
		WriteUChar(writeBuffer,verticesToWrite[index].color.b);
		WriteUChar(writeBuffer,verticesToWrite[index].color.a);

		WriteFloat(writeBuffer,verticesToWrite[index].texCoords.x);
		WriteFloat(writeBuffer,verticesToWrite[index].texCoords.y);

		WriteFloat(writeBuffer,verticesToWrite[index].normals.normal.x);
		WriteFloat(writeBuffer,verticesToWrite[index].normals.normal.y);
		WriteFloat(writeBuffer,verticesToWrite[index].normals.normal.z);

		WriteFloat(writeBuffer,verticesToWrite[index].normals.tangent.x);
		WriteFloat(writeBuffer,verticesToWrite[index].normals.tangent.y);
		WriteFloat(writeBuffer,verticesToWrite[index].normals.tangent.z);

		WriteFloat(writeBuffer,verticesToWrite[index].normals.biTangent.x);
		WriteFloat(writeBuffer,verticesToWrite[index].normals.biTangent.y);
		WriteFloat(writeBuffer,verticesToWrite[index].normals.biTangent.z);
	}

	FILE *file;
	fopen_s(&file , filePath , "wb" );
	fwrite (writeBuffer.data(), sizeof(unsigned char), writeBuffer.size(), file);
	fclose (file);

	return true;
}


bool BufferParser::ReadBool(bool& out_bool)
{
	out_bool = *((bool*)m_scan);
	m_scan += sizeof(bool);
	return true;
}


bool BufferParser::ReadFloat(float& out_float)
{
	out_float = *((float*)m_scan);
	m_scan += sizeof(float);
	return true;
}


bool BufferParser::ReadInt(int& out_int)
{
	out_int = *((int*)m_scan);
	m_scan += sizeof(int);
	return true;
}


bool BufferParser::ReadUInt(unsigned int& out_int)
{
	out_int = *((unsigned int*)m_scan);
	m_scan += sizeof(unsigned int);
	return true;
}


bool BufferParser::ReadString(std::string& out_string)
{
	char data;
	ReadChar(data);
	while(data != '\0')
	{
		out_string.push_back(data);
		ReadChar(data);
	}
	return true;
}


bool BufferParser::ReadChar(char& out_char)
{
	out_char = *((char*)m_scan);
	m_scan += sizeof(char);
	return true;
}


bool BufferParser::ReadUChar(unsigned char& our_char)
{
	our_char = *((unsigned char*)m_scan);
	m_scan += sizeof(unsigned char);
	return true;
}


void BufferParser::Rewind()
{
	m_scan = m_buffer;
}

bool BufferParser::WriteFloat(std::vector<unsigned char>& buffer,const float data)
{
	for(int index=0; index < sizeof(float); index++)
		buffer.push_back(((unsigned char*)&data)[index]);
	
	return true;
}


bool BufferParser::WriteChar(std::vector<unsigned char>& buffer,const char data)
{
	for(int index=0; index < sizeof(char); index++)
		buffer.push_back(((unsigned char*)&data)[index]);

	return true;
}


bool BufferParser::WriteInt(std::vector<unsigned char>& buffer,const int data)
{
	for(int index=0; index < sizeof(int); index++)
		buffer.push_back(((unsigned char*)&data)[index]);

	return true;
}


bool BufferParser::WriteUInt(std::vector<unsigned char>& buffer,const unsigned int data)
{
	for(int index=0; index < sizeof(unsigned int); index++)
		buffer.push_back(((unsigned char*)&data)[index]);

	return true;
}


bool BufferParser::WriteBool(std::vector<unsigned char>& buffer,const bool data)
{
	for(int index=0; index < sizeof(bool); index++)
		buffer.push_back(((unsigned char*)&data)[index]);

	return true;
}


bool BufferParser::WriteString(std::vector<unsigned char>& buffer,const std::string& data)
{
	for(size_t index=0; index < data.length(); index++)
		buffer.push_back(data[index]);
	buffer.push_back('\0');

	return true;
}


bool BufferParser::WriteUChar(std::vector<unsigned char>& buffer,const unsigned char data)
{
	for(int index=0; index < sizeof(unsigned char); index++)
		buffer.push_back(((unsigned char*)&data)[index]);
	
	return true;
}

};