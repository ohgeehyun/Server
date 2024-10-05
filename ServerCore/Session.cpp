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
	//send�� recv�� �ٸ��� �����ҹ���������.
	//1)���� ����
	//2)sendEvent ����? ����? ������? WSASend��ø?

	//TEMP 
	//�ǽð�
	SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this();//ADD_REF
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);
	
	WRITE_LOCK;
	RegisterSend(sendEvent);
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	//TEMP
	wcout << "DISconnect : " <<cause << endl;

	OnDisconnected();
	SocketUtils::Close(_socket);
	GetService()->ReleaseSessioin(GetSessionRef());

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

void Session::RegisterConnect()
{
}

void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init(); //overlapped �� �ʱ�ȭ
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
			//������ �ִ� ��Ȳ
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
	_connected.store(true);

	//���� ���
	GetService()->AddSession(GetSessionRef());

	//������ �ڵ忡�� �����ε�
	OnConnected();

	//���ŵ��
	RegisterRecv();

}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr; //RELEASE_REF
	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}
	//������ �ڵ忡�� �����ε�
	OnRecv(_recvBuffer, numOfBytes);

	//���� ���
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

	//���������� �����ε�
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
