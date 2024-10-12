#pragma once
#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "CoreGlobal.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"

int main()
{
	ClientPacketHandler::Init();
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
	while (true) 
	{
		Protocol::S_TEST pkt;
		pkt.set_id(1000);
		pkt.set_hp(100);
		pkt.set_attack(10);
		{
			Protocol::BuffData* data = pkt.add_buffs();
			data->set_buffid(100);
			data->set_remaintime(2.5f);
			data->add_victims(4000);
		}
		{
			Protocol::BuffData* data = pkt.add_buffs();
			data->set_buffid(200);
			data->set_remaintime(2.5f);
			data->add_victims(1000);
			data->add_victims(3000);
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	
		GSessionManager->Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}
	
	GThreadManager->Join();
}
