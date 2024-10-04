#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	//동일한 쓰레드가 소유하고 있다면 무조건 성공
	//재귀 호출로 인해 소유권을가진상태에서 다시 호출을한상황
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	//아무도 소유 및 공유하고 있지않을 때 소유권을 얻는다.
	const int64 beginTick = GetTickCount64();
	const uint32 desired = ((LThreadId) << 16 & WRITE_THREAD_MASK);
	

	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				//경합에서 승리
				_writeCount++;
				return;
			}
		}
		///5천번 동안 락을 잡을려했으나 못잡음
		//5천번을 돌고 최대시간이상만큼 기다렸으나 빠저나오지못하고있는상황
		//의도적인 crash
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		this_thread::yield(); //자원을 양보하고 처음부터 시작
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	//ReadLock 다 풀기 전에는 WriteUnlock 불가능.
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
	//동일한 쓰레다가 소유하고 있다면 무조건 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	//아무도 소유하고 있지 않을 때 경합해서 공유 카운트 ++
	//즉 아무도 write 하지않을때 read는 사용중이어도 읽기만할떄는 상관이없다. 대신 cout++
	const int64 beginTick = GetTickCount64();
	while (true) 
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			//실패의 경우 누군가 미리 write를 쓰고있었거나 간발의 차이에 뺏겻다.
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
	//READ 하고있는 스레드가 0인데 거기서 1을 뺄려고할경우 lock을 걸기전에 unlock이 된다는소리.
	if((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
		CRASH("MULTIPLE_UNLOCK")
}
