#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	//������ �����尡 �����ϰ� �ִٸ� ������ ����
	//��� ȣ��� ���� ���������������¿��� �ٽ� ȣ�����ѻ�Ȳ
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	//�ƹ��� ���� �� �����ϰ� �������� �� �������� ��´�.
	const int64 beginTick = GetTickCount64();
	const uint32 desired = ((LThreadId) << 16 & WRITE_THREAD_MASK);
	

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				//���տ��� �¸�
				_writeCount++;
				return;
			}
		}
		///5õ�� ���� ���� ������������ ������
		//5õ���� ���� �ִ�ð��̻�ŭ ��ٷ����� �������������ϰ��ִ»�Ȳ
		//�ǵ����� crash
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield(); //�ڿ��� �纸�ϰ� ó������ ����
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	//ReadLock �� Ǯ�� ������ WriteUnlock �Ұ���.
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
	{
		_lockFlag.store(EMPTY_FLAG);
	}

}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	//������ �����ٰ� �����ϰ� �ִٸ� ������ ����
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	//�ƹ��� �����ϰ� ���� ���� �� �����ؼ� ���� ī��Ʈ ++
	//�� �ƹ��� write ���������� read�� ������̾ �б⸸�ҋ��� ����̾���. ��� cout++
	const int64 beginTick = GetTickCount64();
	while (true) 
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			//������ ��� ������ �̸� write�� �����־��ų� ������ ���̿� �����.
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	//READ �ϰ��ִ� �����尡 0�ε� �ű⼭ 1�� �������Ұ�� lock�� �ɱ����� unlock�� �ȴٴ¼Ҹ�.
	if((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTIPLE_UNLOCK")
}
