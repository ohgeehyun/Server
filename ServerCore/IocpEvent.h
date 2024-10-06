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
//CreateIoCompletionPort 의 키 값과 GetQueueCompletionStatus할 때 overlapped를 상속받을 구조체
//그 중 overlapped를 상속받아 read,wirte,accpet등등을 구분하기위해 Event로 좀 더 광범위하게 사용하기위한 class 그렇기에 가상함수를 사용해서는 안된다.
//OVERLAPPED를 상속하여 사용하면 offset 0번으로 OVERLAPPED가 오기떄문에 overlapped포인터로도 사용가능
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
    //TODO accept의 경우 인자가 추가적으로 있을수있다.
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

