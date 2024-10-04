#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore GIocpCore ;

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObjectRef iocpObject)
{
	//Completion port���� �����̳� ���ϸ� ����ϴ� ���� �ƴ� �ٸ� �ϰ��� ����ϰ� ��Ʈ��ũ �Ӹ� �ƴ϶� �پ��ϰ� ��밡��
	//�׷��⿡ ����� �� ���ְ� IocpObject class�� ���� ���
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle,/*key*/0, 0);
	 
}

bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT & numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		IocpObjectRef iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
	}
	else
	{
		//timeout���� ���� Ȯ���� ���� Ȯ��
		int32 errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			//TODO : �α� ���
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}
