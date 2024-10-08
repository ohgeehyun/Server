#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"

GameSessionManager* GSessionManager;

void GameSession::OnConnected()
{
	GSessionManager->Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisConnected()
{
	GSessionManager->Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
	//Echo
	//cout << "Content Recv Len = " << len << endl;
	ServerPacketHandler::HandlePacket(buffer, len);
}

void GameSession::OnSend(int32 len)
{
	//cout << "Content Send Len = " << len << endl;
}