#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"

ThreadManager* GThreadManager = nullptr;
Memory* GMemory = nullptr;
SendBufferManager* GSendBufferManager;
DeadLockProfiler* GDeadLockProfiler = nullptr;


class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GMemory = new Memory();
		GSendBufferManager = new SendBufferManager();
		GDeadLockProfiler = new DeadLockProfiler();
		SocketUtils::Init();
	};
	~CoreGlobal() 
	{
		delete GThreadManager;
		delete GMemory;
		delete GSendBufferManager;
		delete GDeadLockProfiler;
		SocketUtils::Clear();
	};

}GCoreGlobal;

