#pragma once
#include "Allocator.h"

class MemoryPool;

/*----------------
	 Memory
-----------------*/
class Memory
{
	//메모리를 관리하다보면 메모리가 작은애들은 촘촘하게 할당하게되는데 비교적 큰데이터들은 커지면 커질수록 겹칠확률이 적기때문에
	//사이즈가 클수록 메모리pool갯수를 줄여서 유동적으로
	enum
	{
		//~1024까지 32단위 , 2048까지 128단위, 4096까지 256단위
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};
public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	vector<MemoryPool*> _pools;
	//메모리 크기 <-> 메모리 풀
	//0(1)의 복잡도를 위해 테이블
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename ...Args>
//우측값 참조와 완벽한전달
//&의 경우 좌측값 참조로 어느 메모리에할당되어있느 값을 참조
//&&의 경우 우측값 참조로 메모리에할당되어있지않은 임시값
//Args는 가변인자 템플릿이다. 여러 인자를 받기위해 사용
Type* xnew(Args&&...args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	//여기까지만 한다면 메모리는 할당해주지만 class의경우 생성자와 소멸자가있는데 그것을 자동으로 호출해주진않는다.
	//placement new 라는 것이 메모리를 할당하고 생성자를 호출하여준다.
	//메모리는 할당되어있으니 생성자를 호출해달라고 하는 것
	//인자가 있는 경우의 생성자도 있을 것이다.c++11이전에는 인자의 타입마다 하나하나 선언했었다고한다.(노가다)
	new(memory)Type(forward<Args>(args)...);

	return memory;
};

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	PoolAllocator::Release(obj);
}

template<typename Type,typename...Args>
shared_ptr<Type> Make_shared(Args&&... args)
{
	return shared_ptr<Type>{xnew<Type>(forward<Args>(args)...),xdelete<Type>};
}

