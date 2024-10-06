#pragma once
/*--------------------------
		Base Allocator
----------------------------*/

using namespace std;;

class BaseAllocator
{
public:
	static void* Alloc(int32 size);
	static void  Release(void* ptr);


private:
};
/*----------------------------
		StompAllocator
------------------------------*/

class StompAllocator
{
	//�ּ� ����������4kb SYSTEM_INFOŬ����������Ͽ� �ü������ ����ϴ� �ּ� �޸� �������� Ȯ��
	enum {PAGE_SIZE = 0x1000};

public:
	static void* Alloc(int32 size);
	static void  Release(void* ptr);
};
/*----------------------
	PoolAllocator
-------------------------*/
class PoolAllocator
{
	//�ּ� ����������
	enum { PAGE_SIZE = 0x1000 };

public:
	static void* Alloc(int32 size);
	static void  Release(void* ptr);
};

/*----------------------------
		STL Allocator
------------------------------*/
template<typename T>
class StlAllocator {
public:
	using value_type = T;
	StlAllocator() {};

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) {};

	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(size));

	}

	void deallocate(T* ptr, size_t count)
	{
		PoolAllocator::Release(ptr);
	}

};

