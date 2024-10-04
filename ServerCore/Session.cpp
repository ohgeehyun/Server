#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
/*---------------
	 Session
-----------------*/


Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
	//TODO
}