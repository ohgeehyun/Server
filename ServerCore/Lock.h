#pragma once
#include "Types.h"

//기본 lockgaurd 라는 기능이있는데 굳이 class로 만드는이유
//기본적으로 재귀적으로 lock을걸수없다 
/*
	lock(m);
	function(); <-내부에 락이 있는 function
	이 경우 락을 잡을수없고 이것은 다른 lock으로 해결이가능하다고는하지만
	경우에 따라 상호베타적인 경우가 발생할수있다.

	또한 데이터를 읽을때는 큰 문제가없지만 주로 데이터를 write할때 문제가 생긴다.
	데이터를 읽기만할때도 계속 lock을 잠그는 것이 좋은현상 은 아니기떄문	
*/


/*--------------------
	RW SpinLock
	기본 적인 읽고 쓰기 라 금방처리되는 일이기 때문에 컨텍스트 스위칭 보다는 spin을 사용 
	W: WriteFlag(Exclusive Lock Owner ThreadId)
	R: ReadFlag(Shared Lock Count)
	[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
	32bit 정수형에 앞부분엔 writeflag에 대한 정보 뒷부분엔 readflag에 대한 정보 를 담아둔 비트플래그 사용방식
---------------------*/
// W->R Write에서 Read 호출은 가능
// R->W Read에서 Write 호출은 불가능 
// -이 락에서 사용될 정책-
class Lock
{
	enum : uint32
	{
		ACQUIRE_TIMEOUT_TICK = 10000, //최대로 기다려줄 시간
		MAX_SPIN_COUNT = 5000, // 최대 스핀 횟수
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