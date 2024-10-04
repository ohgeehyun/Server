#pragma once
#include "pch.h"
#include <atomic>
#include <future>
#include "ThreadManager.h"
#include "SocketUtils.h"
#include "Listener.h"

#include "Service.h"
#include "Session.h"

int main()
{
	ServerServiceRef service = Make_shared<ServerService>(
		NetAddress(L"127.0.0.1", 5252),
		Make_shared<IocpCore>(),
		Make_shared<Session>,//TODO : SessionManager 등등
		100);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
		{
			while (true)
			{
				service->GetIocpCore()->Dispatch();
			}
		});


	}
	GThreadManager->Join();
	cout << "client Connected!" << endl;

}
