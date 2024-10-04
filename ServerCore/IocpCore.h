#pragma once
/*------------------
     IocpObject
--------------------*/
//CreateIoCompletionPort �� Ű ���� GetQueueCompletionStatus�� �� overlapped�� ��ӹ��� ����ü
//���� key���� ������� class
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:

	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) abstract;
};



/*-------------------
	IocpCore
-------------------*/
class IocpCore
{

public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	bool Register(IocpObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);//��Ȳ�� ���� GetQueueCompletionStatus ȣ��

private:
	HANDLE _iocpHandle;

};



