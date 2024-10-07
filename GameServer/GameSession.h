#pragma once
#include "Session.h"

class GameSession : public Session
{
public:
	virtual void OnConnected() override;
	virtual void OnDisConnected() override;
	virtual int32 OnRecv(BYTE* buffer, int32 len);
	virtual void OnSend(int32 len) override;

};

