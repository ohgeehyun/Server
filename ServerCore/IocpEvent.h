#pragma once
/*------------------
     IocpEvent
--------------------*/
class Session;
enum class EventType : uint8
{
    Connect,
    DisConnect,
    Accept,
    //PreRecv 
    Recv,
    Send
};

/*------------------
     IocpEvent
--------------------*/
//CreateIoCompletionPort �� Ű ���� GetQueueCompletionStatus�� �� overlapped�� ��ӹ��� ����ü
//�� �� overlapped�� ��ӹ޾� read,wirte,accpet����� �����ϱ����� Event�� �� �� �������ϰ� ����ϱ����� class �׷��⿡ �����Լ��� ����ؼ��� �ȵȴ�.
//OVERLAPPED�� ����Ͽ� ����ϸ� offset 0������ OVERLAPPED�� ���⋚���� overlapped�����ͷε� ��밡��
class IocpEvent : public OVERLAPPED
{
public:
    IocpEvent(EventType type);

    void      Init();
public:
    EventType eventType;
    IocpObjectRef owner;
};

/*------------------
    ConnectEvent
--------------------*/
class ConnectEvent : public IocpEvent
{
public:
    ConnectEvent() :IocpEvent(EventType::Connect) {}
};
/*------------------
    DisConnectEvent
--------------------*/
class DisConnectEvent : public IocpEvent
{
public:
    DisConnectEvent() :IocpEvent(EventType::DisConnect) {}
};

/*------------------
    AcceptEvent
--------------------*/
class AcceptEvent : public IocpEvent
{
public:
    AcceptEvent() :IocpEvent(EventType::Accept) {}

public:
    //TODO accept�� ��� ���ڰ� �߰������� �������ִ�.
    SessionRef session = nullptr;
};

/*------------------
    RecvEvent
--------------------*/
class RecvEvent : public IocpEvent
{
public:
    RecvEvent() :IocpEvent(EventType::Recv) {}
};


/*------------------
    SendEvent
--------------------*/
class SendEvent : public IocpEvent
{
public:
    SendEvent() :IocpEvent(EventType::Send) {}

    //TEMP
    vector<BYTE> buffer;
};

