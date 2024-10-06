#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
/*---------------
	 Session
-----------------*/


Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
	//send는 recv와 다르게 생각할문제가많다.
	//1)버퍼 관리
	//2)sendEvent 관리? 단일? 여러개? WSASend중첩?

	//TEMP 
	//실시간
	SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this();//ADD_REF
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);
	
	WRITE_LOCK;
	RegisterSend(sendEvent);
}

bool Session::Connect()
{
	//분산 서버의 경우 서버랑 서버끼리 통신할 일도 있다.
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	//TEMP
	wcout << "DISconnect : " <<cause << endl;

	OnDisConnected();
	GetService()->ReleaseSessioin(GetSessionRef());

	RegisterDisConnect();

}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::DisConnect:
		ProcessDisConnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfByte);
		break;
	case EventType::Send:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfByte);
		break;
	default:
		break;
	}


}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client)
		return false;

	if(SocketUtils::BindAnyAddress(_socket,0/*남는포트*/ ))

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();//ADD_REF

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr();//붙어야하는 쪽 주소

	if(false ==SocketUtils::connectEx(_socket,reinterpret_cast<SOCKADDR*>(&sockAddr),sizeof(sockaddr),nullptr,0,&numOfBytes,&_connectEvent));
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr; //RELEASE REF
			return false;
		}
	}
}

bool Session::RegisterDisConnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr; //RELEASE_REF
			return false;
		}
	}

	return true;
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init(); //overlapped 값 초기화
	_recvEvent.owner = shared_from_this(); //ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
	wsaBuf.len = len32(_recvBuffer);

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			//문제가 있는 상황
			HandleError(errorCode);
			_recvEvent.owner = nullptr;//RELEASE_REF
		}
	}
}

void Session::RegisterSend(SendEvent* sendEvent)
{
	if (IsConnected() == false)
		return;

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR ==::WSASend(_socket, &wsaBuf, 1, OUT & numOfBytes, 0, sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSAGetLastError())
		{
			HandleError(errorCode);
			sendEvent->owner = nullptr; //RELESE_REF
			xdelete(sendEvent);
		}
	}

}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;//RELEASE_REF

	_connected.store(true);

	//세션 등록
	GetService()->AddSession(GetSessionRef());

	//컨텐츠 코드에서 재정의
	OnConnected();

	//수신등록
	RegisterRecv();

}

void Session::ProcessDisConnect()
{
	_disconnectEvent.owner = nullptr;;//RELEASE_REF

}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr; //RELEASE_REF
	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}
	//컨텐츠 코드에서 재정의
	OnRecv(_recvBuffer, numOfBytes);

	//수신 등록
	RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent,int32 numOfBytes)
{
	sendEvent->owner = nullptr; //RELEASE_REF
	xdelete(sendEvent);

	if (numOfBytes == 0)
	{
		Disconnect(L"SEND 0");
		return;
	}

	//콘텐츠에서 재정의
	OnSend(numOfBytes);
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;
	default:
		//TODO : log
		cout << "Handle Error : " << errorCode << endl;
		break;
	}
}
