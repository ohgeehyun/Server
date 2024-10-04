#include "pch.h"
#include "Allocator.h"
#include "Memory.h"

void* BaseAllocator::Alloc(int32 size)
{
	return malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	free(ptr);
}


void* StompAllocator::Alloc(int32 size)
{
	//4096+4095 / 4096을 하면 1.9999가 나오는데 정수는 어짜피  버림이니 1이 나온다.
	//반올림 코드인데 알고리즘문제에서 자주나오니 찾아보자
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const int64 dataOffset = pageCount * PAGE_SIZE - size;
	//메모리 언더플로우
	void* baseAddress = VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
}
void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address - (address % PAGE_SIZE);
	VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}

void* PoolAllocator::Alloc(int32 size)
{

	return GMemory->Allocate(size);
}
void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}