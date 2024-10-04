#pragma once
#include "Allocator.h"

class MemoryPool;

/*----------------
	 Memory
-----------------*/
class Memory
{
	//�޸𸮸� �����ϴٺ��� �޸𸮰� �����ֵ��� �����ϰ� �Ҵ��ϰԵǴµ� ���� ū�����͵��� Ŀ���� Ŀ������ ��ĥȮ���� ���⶧����
	//����� Ŭ���� �޸�pool������ �ٿ��� ����������
	enum
	{
		//~1024���� 32���� , 2048���� 128����, 4096���� 256����
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
	//�޸� ũ�� <-> �޸� Ǯ
	//0(1)�� ���⵵�� ���� ���̺�
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename ...Args>
//������ ������ �Ϻ�������
//&�� ��� ������ ������ ��� �޸𸮿��Ҵ�Ǿ��ִ� ���� ����
//&&�� ��� ������ ������ �޸𸮿��Ҵ�Ǿ��������� �ӽð�
//Args�� �������� ���ø��̴�. ���� ���ڸ� �ޱ����� ���
Type* xnew(Args&&...args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	//��������� �Ѵٸ� �޸𸮴� �Ҵ��������� class�ǰ�� �����ڿ� �Ҹ��ڰ��ִµ� �װ��� �ڵ����� ȣ���������ʴ´�.
	//placement new ��� ���� �޸𸮸� �Ҵ��ϰ� �����ڸ� ȣ���Ͽ��ش�.
	//�޸𸮴� �Ҵ�Ǿ������� �����ڸ� ȣ���ش޶�� �ϴ� ��
	//���ڰ� �ִ� ����� �����ڵ� ���� ���̴�.c++11�������� ������ Ÿ�Ը��� �ϳ��ϳ� �����߾��ٰ��Ѵ�.(�밡��)
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

