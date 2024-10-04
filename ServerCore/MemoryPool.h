#pragma once


enum
{
	SLIST_ALIGNMENT = 16
};

//�޸�Ǯ�� �����Ҷ� ������ �����ͳ��� ������ ���̳� �������� �����ͳ��� �Ұ��̳� �� ���ؾ��Ѵ�.
//[32] [64] [] [] [] [] []
//[][]:�޸�Ǯ

/*------------------------------------
			  MemoryHeader
-------------------------------------*/
DECLSPEC_ALIGN(SLIST_ALIGNMENT) 
struct MemoryHeader : public SLIST_ENTRY
{
	//[MemoryHeader][Data]
	MemoryHeader(int32 size) : allocSize(size) {};

	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size); //placement new
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetacchHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int32 allocSize;
	//TODO :�ʿ��� �߰� ����
};

/*------------------------------------
			  MemoryPool
-------------------------------------*/
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void Push(MemoryHeader* ptr);
	MemoryHeader* Pop();

private:
	SLIST_HEADER _header;
	int32 _allocsize = 0;
	atomic<int32>_useCount = 0;//�޸��� ���� Pool�� ī���;ƴ�
	atomic<int32>_reserveCount = 0;
};

