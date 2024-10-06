#pragma once
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"

class GameSession : public Session
{
public:
	virtual int32 OnRecv(BYTE* buffer, int32 len) override
	{
		//Echo
		cout << "Content Recv Len = " << len << endl;
		Send(buffer, len);
		return len;
	}
	virtual void OnSend(int32 len) override
	{
		cout << "Content Send Len = " << len << endl;
	}
};


int main()
{
	ServerServiceRef service = Make_shared<ServerService>(
		NetAddress(L"127.0.0.1", 5252),
		Make_shared<IocpCore>(),
		Make_shared<GameSession>,//TODO : SessionManager 등등
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
