#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
/*---------------
	 Session
-----------------*/


Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}


void Session::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool registerSend = false;

	{
		//현재 RegisterSend가 걸리지 않은 상태라면 , 걸어준다.
		WRITE_LOCK;

		_sendQueue.push(sendBuffer);

		if (_sendRegistered.exchange(true) == false)
			registerSend = true;
	}
	if(registerSend)
		RegisterSend();
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
		ProcessSend(numOfByte);
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
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

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

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init(); //overlapped 값 초기화
	_sendEvent.owner = shared_from_this(); //ADD_REF

	//보낼 데이터를 sendEvent에 등록
	{
		WRITE_LOCK;

		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();
			
			writeSize += sendBuffer->WriteSize();
			//TODO: 예외 체크

			_sendQueue.pop();
			_sendEvent.sendbuffers.push_back(sendBuffer);
		}

	}

	//Scatter-Gather (흩어져 있 는 데이터들을 모아서 한방에 보내는 기법)
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendbuffers.size());

	for (SendBufferRef sendBuffer : _sendEvent.sendbuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR ==::WSASend(_socket, wsaBufs.data(),static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, 0, &_sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSAGetLastError())
		{
			HandleError(errorCode);
			_sendEvent.owner = nullptr; //RELESE_REF
			_sendEvent.sendbuffers.clear();
			_sendRegistered.store(false);
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

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize();
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), numOfBytes);
	if (processLen <0 || dataSize < processLen || _recvBuffer.OnRead(processLen)==false )
	{
		Disconnect(L"OnRead Overflow");
		return;
	}

	//read write커서 초기화
	_recvBuffer.Clean();
	//수신 등록
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.owner = nullptr; //RELEASE_REF
	_sendEvent.sendbuffers.clear();

	if (numOfBytes == 0)
	{
		Disconnect(L"SEND 0");
		return;
	}

	//콘텐츠에서 재정의
	OnSend(numOfBytes);

	WRITE_LOCK;
	if (_sendQueue.empty())
		_sendRegistered.store(false);
	else
		RegisterSend();
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
