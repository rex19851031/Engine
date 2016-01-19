#include "MemoryAllocatePool.hpp"

#include <stdlib.h> 
#include "Engine\Renderer\OpenGLRenderer.hpp"
#include "Engine\Core\HenryFunctions.hpp"

// #undef DEBUG_NEW 
// #undef new

#define UNUSED(x) (void)(x);


// void* operator new(size_t size)
// {
// 	if (!Henry::_memoryAllocatePool)
// 	{
// 		size_t halfGB = 524288000;//1073741824;
// 		Henry::_memoryAllocatePool = (Henry::MemoryAllocatePool*)malloc(sizeof(Henry::MemoryAllocatePool));
// 		Henry::_memoryAllocatePool->Initialize(halfGB);
// 	}
// 
// 	return Henry::_memoryAllocatePool->AllocateMemory(size);
// };
// 
// 
// void operator delete(void *p)
// {
// 	Henry::_memoryAllocatePool->FreeMemory(p);
// };
// 
// 
// void* operator new[](size_t size)
// {
// 	if (!Henry::_memoryAllocatePool)
// 	{
// 		size_t halfGB = 524288000;//1073741824;
// 		Henry::_memoryAllocatePool = (Henry::MemoryAllocatePool*)malloc(sizeof(Henry::MemoryAllocatePool));
// 		Henry::_memoryAllocatePool->Initialize(halfGB);
// 	}
// 
// 	return Henry::_memoryAllocatePool->AllocateMemory(size);
// };
// 
// 
// void operator delete[](void* p)
// {
// 	Henry::_memoryAllocatePool->FreeMemory(p);
// };
// 
// 
// void* operator new(size_t size, const char* file, int line)
// {
// 	if (!Henry::_memoryAllocatePool)
// 	{
// 		size_t halfGB = 524288000;//1073741824;
// 		Henry::_memoryAllocatePool = (Henry::MemoryAllocatePool*)malloc(sizeof(Henry::MemoryAllocatePool));
// 		Henry::_memoryAllocatePool->Initialize(halfGB);
// 	}
// 
// 	return Henry::_memoryAllocatePool->AllocateMemory(size, file, line);
// };
// 
// 
// void operator delete(void* p, const char* file, int line)
// {
// 	Henry::_memoryAllocatePool->FreeMemory(p, file, line);
// };
// 
// 
// void* operator new[](size_t size, const char* file, int line)
// {
// 	if (!Henry::_memoryAllocatePool)
// 	{
// 		size_t halfGB = 524288000;//1073741824;
// 		Henry::_memoryAllocatePool = (Henry::MemoryAllocatePool*)malloc(sizeof(Henry::MemoryAllocatePool));
// 		Henry::_memoryAllocatePool->Initialize(halfGB);
// 	}
// 
// 	return Henry::_memoryAllocatePool->AllocateMemory(size, file, line);
// };
// 
// 
// void operator delete[](void* p, const char* file, int line)
// {
// 	Henry::_memoryAllocatePool->FreeMemory(p, file, line);
// };


namespace Henry
{

MemoryAllocatePool* _memoryAllocatePool = nullptr;

MemoryAllocatePool::MemoryAllocatePool()
{
//	_memoryAllocatePool = this;
// 	m_beginOfMemory = (void*)malloc(numOfBytes);
// 	m_headOfFreeSpace = new FreeSpaceHeader(numOfBytes);
// 	m_headOfFreeSpace = (FreeSpaceHeader*)m_beginOfMemory;
// 	m_tailOfFreeSpace = (FreeSpaceHeader*)m_beginOfMemory;
}


MemoryAllocatePool::~MemoryAllocatePool()
{
	free(m_beginOfMemory);
}


void* MemoryAllocatePool::AllocateMemory(size_t sizeRequired, const char* filename , int line)
{
 	MemorySpaceHeader* currentNode = m_headOfFreeSpace;
	void* return_ptr = m_headOfFreeSpace;
	
	++m_numberOfMemoryBlocks;
	m_allocatedSize += sizeRequired;

	while (currentNode)
 	{
		if (!currentNode->available && m_largestSizeOfMemoryAllocated < currentNode->sizeInBytes)
			m_largestSizeOfMemoryAllocated = sizeRequired;

		if (currentNode->sizeInBytes >= sizeRequired)
		{
			if (currentNode == m_tailOfMemory)
			{
				m_allocatedSize += sizeof(MemorySpaceHeader);
				return_ptr = currentNode + sizeof(MemorySpaceHeader);

				MemorySpaceHeader* tail = currentNode + sizeof(MemorySpaceHeader) + sizeRequired;
				tail->prev = currentNode;
				tail->next = nullptr;
				tail->sizeInBytes = currentNode->sizeInBytes - sizeof(MemorySpaceHeader) - sizeRequired;

				currentNode->filename = filename;
				currentNode->line = line;
				currentNode->available = false;
				currentNode->sizeInBytes = sizeRequired;
				currentNode->next = tail;
				m_tailOfMemory = tail;
				break;
			}
			else if (currentNode->available)
			{
				return_ptr = currentNode + sizeof(MemorySpaceHeader);
				currentNode->available = false;
				currentNode->filename = filename;
				currentNode->line = line;

				int spaceRemain = currentNode->sizeInBytes - sizeRequired;
				if (spaceRemain > sizeof(MemorySpaceHeader))
				{
					m_allocatedSize += sizeof(MemorySpaceHeader);

					MemorySpaceHeader* space = currentNode + sizeof(MemorySpaceHeader) + sizeRequired;
					space->available = true;
					space->next = currentNode->next;
					space->prev = currentNode;
					space->sizeInBytes = spaceRemain - sizeof(MemorySpaceHeader);

					currentNode->sizeInBytes = sizeRequired;
					currentNode->next = space;
				}
				else
					m_allocatedSize += spaceRemain;

				break;
			}
		}

 		currentNode = currentNode->next;
 	}

	if (currentNode == nullptr)
		throw std::bad_alloc();

	return return_ptr;
}


void MemoryAllocatePool::FreeMemory(void* p, const char* filename, int line)
{
	UNUSED(filename);
	UNUSED(line);
	
	MemorySpaceHeader* ptr = (MemorySpaceHeader*)p - sizeof(MemorySpaceHeader);
	ptr->available = true;

	if (m_largestSizeOfMemoryAllocated == ptr->sizeInBytes)
		m_largestSizeOfMemoryAllocated = 0;
	
	--m_numberOfMemoryBlocks;
	m_allocatedSize -= ptr->sizeInBytes;

	if (ptr->next != nullptr)
	{
		MemorySpaceHeader* nextSpace = ptr->next;
		if (nextSpace->available)
		{
			m_allocatedSize -= sizeof(MemorySpaceHeader);

			ptr->sizeInBytes += (nextSpace->sizeInBytes + sizeof(MemorySpaceHeader));
			ptr->next = nextSpace->next;
			if (ptr->next != nullptr)
				ptr->next->prev = ptr;
			else
				m_tailOfMemory = ptr;
		}
	}

	if (ptr->prev != nullptr)
	{
		MemorySpaceHeader* previousSpace = ptr->prev;
		if (previousSpace->available)
		{
			m_allocatedSize -= sizeof(MemorySpaceHeader);
			
			previousSpace->sizeInBytes += (ptr->sizeInBytes + sizeof(MemorySpaceHeader));
			previousSpace->next = ptr->next;
			if (previousSpace->next != nullptr)
				previousSpace->next->prev = previousSpace;
			else
				m_tailOfMemory = previousSpace;
		}
	}
}


void MemoryAllocatePool::ScanMemory()
{
	MemorySpaceHeader* currentNode = m_headOfFreeSpace;
	while (currentNode)
	{
		if (!currentNode->available)
		{
			DebuggerPrintf("%s(%d) : memory leak , size-> %d bytes , memory address -> %d \r\n", currentNode->filename, currentNode->line, currentNode->sizeInBytes, currentNode);
		}
		currentNode = currentNode->next;
	}
}


void MemoryAllocatePool::Initialize(size_t numberOfBytes)
{
	m_beginOfMemory = malloc(numberOfBytes);
	m_memorySize = numberOfBytes;
	m_allocatedSize = sizeof(MemorySpaceHeader);
	m_numberOfMemoryBlocks = 0;
	m_largestSizeOfMemoryAllocated = 0;

	if (m_beginOfMemory == nullptr)
		throw std::bad_alloc();

	m_headOfMemory = (MemorySpaceHeader*)m_beginOfMemory;
	m_headOfMemory->available = true;
	m_headOfMemory->prev = nullptr;
	m_headOfMemory->next = nullptr;
	m_headOfMemory->nextSpac = nullptr;
	m_headOfMemory->sizeInBytes = numberOfBytes - sizeof(MemorySpaceHeader);

	m_tailOfMemory = m_headOfMemory;
	m_headOfFreeSpace = m_headOfMemory;
}


void MemoryAllocatePool::Render()
{
// 	OpenGLRenderer::BindWhiteTexture();
// 
// 	std::vector<Vertex_PosColor> drawVertices;
// 	Vec2f minPosition(1000, 50);
// 	Vec2f maxPosition(800, 650);
// 	Vertex_PosColor vertices[4];
// 	float length = maxPosition.y - minPosition.y;
// 	float oneOverMemorySize = 1.0f / (float)m_memorySize;
// 	int memoryAccumulate = 0;
// 
// 	MemorySpaceHeader* currentNode = m_headOfFreeSpace;
// 	while (currentNode)
// 	{
// 		memoryAccumulate += currentNode->sizeInBytes;
// 
// 		vertices[0].position = Vec3f(minPosition.x, minPosition.y, 0.0f);
// 		vertices[1].position = Vec3f(minPosition.x, maxPosition.y, 0.0f);
// 		vertices[2].position = Vec3f(maxPosition.x, maxPosition.y, 0.0f);
// 		vertices[3].position = Vec3f(maxPosition.x, minPosition.y, 0.0f);
// 
// 		RGBA color(1.0f, 1.0f, 1.0f, 1.0f);
// 		if (currentNode->available)
// 			color = RGBA(1.0f, 1.0f, 0.0f, 1.0f);
// 
// 		for (int index = 0; index < 4; ++index)
// 		{
// 			vertices->color = color;
// 			drawVertices.push_back(vertices[index]);
// 		}
// 
// 		currentNode = currentNode->next;
// 	}
// 
// 	OpenGLRenderer::DrawVertexWithVertexArray2D(drawVertices, OpenGLRenderer::SHAPE_QUADS);
}


};