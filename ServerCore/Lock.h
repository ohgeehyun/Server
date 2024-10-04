#pragma once
#include "Types.h"

//�⺻ lockgaurd ��� ������ִµ� ���� class�� ���������
//�⺻������ ��������� lock���ɼ����� 
/*
	lock(m);
	function(); <-���ο� ���� �ִ� function
	�� ��� ���� ���������� �̰��� �ٸ� lock���� �ذ��̰����ϴٰ��������
	��쿡 ���� ��ȣ��Ÿ���� ��찡 �߻��Ҽ��ִ�.

	���� �����͸� �������� ū ������������ �ַ� �����͸� write�Ҷ� ������ �����.
	�����͸� �б⸸�Ҷ��� ��� lock�� ��״� ���� �������� �� �ƴϱ⋚��	
*/


/*--------------------
	RW SpinLock
	�⺻ ���� �а� ���� �� �ݹ�ó���Ǵ� ���̱� ������ ���ؽ�Ʈ ����Ī ���ٴ� spin�� ��� 
	W: WriteFlag(Exclusive Lock Owner ThreadId)
	R: ReadFlag(Shared Lock Count)
	[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
	32bit �������� �պκп� writeflag�� ���� ���� �޺κп� readflag�� ���� ���� �� ��Ƶ� ��Ʈ�÷��� �����
---------------------*/
// W->R Write���� Read ȣ���� ����
// R->W Read���� Write ȣ���� �Ұ��� 
// -�� ������ ���� ��å-
class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000, //�ִ�� ��ٷ��� �ð�
		MAX_SPIN_COUNT = 5000, // �ִ� ���� Ƚ��
		WRITE_THREAD_MASK = 0xFFFF'0000, //writeflag
		READ_COUNT_MASK = 0x0000'FFFF, //readflag
		EMPTY_FLAG = 0x0000'0000
	};

public:
	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	Atomic<uint32> _lockFlag;
	uint16 _writeCount = 0;
};

/*--------------
	LockGuards
----------------*/
class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock,const char* name) : _lock(lock),_name(name) { _lock.ReadLock(_name); }
	~ReadLockGuard() { _lock.ReadUnlock(_name); }
private:
	Lock& _lock;
	const char* _name;
};


class WriteLockGuard
{

public:
	WriteLockGuard(Lock& lock, const char* name) : _lock(lock),_name(name) { _lock.WriteLock(_name); }
	~WriteLockGuard() { _lock.WriteUnlock(_name); }

private:
	Lock& _lock;
	const char* _name;
};