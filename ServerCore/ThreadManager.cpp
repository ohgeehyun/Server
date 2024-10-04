#include "pch.h"
#include "ThreadManager.h"


ThreadManager::ThreadManager() 
{
	//main Thread;
	initTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock);
	_threads.push_back(thread([=]()
	{
		initTLS();
		callback();
		DestroyTLS();
	}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::initTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
}

