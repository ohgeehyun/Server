#pragma once
#include "Session.h"

class GameSession : public PacketSession
{
public:
	virtual void OnConnected() override;
	virtual void OnDisConnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len);
	virtual void OnSend(int32 len) override;

public:
	Vector<PlayerRef> _players;

};

