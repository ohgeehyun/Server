#pragma once
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "CoreGlobal.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"

int main()
{
	 GSessionManager = new GameSessionManager();

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
	char sendData[] = "Hello World";
	while (true)
	{
		vector<BuffData> buffs{ BuffData {100, 1.5f}, BuffData{200, 2.3f}, BuffData {300, 0.7f } };
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1000, 100, 10, buffs);

		GSessionManager->Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}
	GThreadManager->Join();
}
