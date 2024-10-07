#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"


GameSessionManager* GSessionManager;

void GameSession::OnConnected()
{
	GSessionManager->Add(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisConnected()
{
	GSessionManager->Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
	//Echo
	cout << "Content Recv Len = " << len << endl;

	SendBufferRef sendBuffer = Make_shared<SendBuffer>(4096);
	sendBuffer->CopyData(buffer, len);

	for(int32 i=0; i<5; i++)
	GSessionManager->Broadcast(sendBuffer);

	return len;
}

void GameSession::OnSend(int32 len)
{
	cout << "Content Send Len = " << len << endl;
}