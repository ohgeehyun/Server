#pragma once
#include"pch.h"
#include "Service.h"
#include "Session.h"
#include "ThreadManager.h"


char sendbuffer[] = "Hello World";

class ServerSession : public Session
{
public:
	virtual void OnConnected() override
	{
		cout << "Connected To Server" << endl;
		Send((BYTE*)sendbuffer, sizeof(sendbuffer));
	}

	virtual int32 OnRecv(BYTE* buffer, int32 len) override
	{
		//Echo
		cout << "Content Recv Len = " << len << endl;

		this_thread::sleep_for(1s);

		Send((BYTE*)sendbuffer, sizeof(sendbuffer));
		return len;
	}
	virtual void OnSend(int32 len) override
	{
		cout << "Content Send Len = " << len << endl;
	}
	virtual void OnDisConnected() override
	{
		cout << "Disconnected" << endl;
	}
};

int main()
{
	this_thread::sleep_for(1s);

	ClientServiceRef service = Make_shared<ClientService>(
		NetAddress(L"127.0.0.1", 5252),
		Make_shared<IocpCore>(),
		Make_shared<ServerSession>,//TODO : SessionManager 등등
		1);
	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++)
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
	cout << "Server Connected!" << endl;

}