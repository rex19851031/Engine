#pragma once

#ifndef MEMORYALLOCATEPOOL_HPP 
#define MEMORYALLOCATEPOOL_HPP

// void* operator new(size_t size);
// void operator delete(void * p);
// 
// void* operator new[](size_t size);
// void operator delete[](void* ptr);
// 
// void* operator new(size_t size, const char* file, int line);
// void operator delete(void* p, const char* file, int line);
// 
// void* operator new[](size_t size, const char* file, int line);
// void operator delete[](void* p, const char* file, int line);

#include <new>
#include "Engine\Core\VertexStruct.hpp"

// #define DEBUG_NEW new(__FILE__, __LINE__)
// #define new DEBUG_NEW


namespace Henry
{


struct MemorySpaceHeader
{
	bool available;
	const char* filename;
	size_t line;
	size_t sizeInBytes;
	MemorySpaceHeader* prev;
	MemorySpaceHeader* next;
	MemorySpaceHeader* nextSpac;
};


class MemoryAllocatePool
{
public:
	MemoryAllocatePool();
	~MemoryAllocatePool();
	void Initialize(size_t numberOfBytes);
	void* AllocateMemory(size_t sizeRequired, const char* filename = "", int line = 0);
	void FreeMemory(void* p, const char* filename = "", int line = 0);
	void ScanMemory();
	void Render();

public:
	size_t m_memorySize;
	size_t m_allocatedSize;
	size_t m_numberOfMemoryBlocks;
	size_t m_largestSizeOfMemoryAllocated;

	void* m_beginOfMemory;
	MemorySpaceHeader* m_headOfMemory;
	MemorySpaceHeader* m_tailOfMemory;
	MemorySpaceHeader* m_headOfFreeSpace;
};

extern MemoryAllocatePool* _memoryAllocatePool;

};

#endif