#pragma once
#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include "functional"


enum class ServiceType : uint8
{
	Server,
	Client,
};

/*------------------
	   Service 
------------------*/

//함수객체
using SessionFactory = function<SessionRef(void)>;

class Service : public enable_shared_from_this<Service>
{
public:
	Service(ServiceType  type, NetAddress address, IocpCoreRef core,SessionFactory factory , /*동접자*/int32 maxSessionCount =1 );
	virtual ~Service();

	virtual bool	Start() abstract;//서비스를 상속받은 곳에서 어떻게 서비스할지모르니 반드시 start부분을 구현하게 만듬
	bool			CanStart() { return _sessionFactory != nullptr; };

	virtual void	CloseService();
	void			SetSessionFatory(SessionFactory func) { _sessionFactory = func; };

	SessionRef		CreateSession();
	void			AddSession(SessionRef session);
	void			ReleaseSessioin(SessionRef session);
	int32			GetCurrentSessionCount() { return _sessionCount; };
	int32			GetMaxSessionCount() { return _maxSessionCount; };
public:
	ServiceType		GetServiceType() { return _type; };
	NetAddress		GetNetAddress() { return _netAddress; };
	IocpCoreRef&	GetIocpCore() { return _iocpCore; };//자주사용되어 혹시라도 부하가많이 갈까봐 일단 참조값으로 생성

protected:
	USE_LOCK;

	ServiceType		_type;
	NetAddress		_netAddress = {};
	IocpCoreRef		_iocpCore;

	Set<SessionRef> _sessions; //연결된 세션들
	int32			_sessionCount = 0;
	int32			_maxSessionCount = 0;
	SessionFactory  _sessionFactory;//세션을 생성하는 함수를 받아줄 일종의 함수포인터

};

/*------------------
	ClientService
------------------*/

class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ClientService() {};

	virtual bool Start() override;

};


/*------------------
    ServerService
------------------*/
class ServerService : public Service
{
public:
	ServerService(NetAddress Address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual		~ServerService() {};

	virtual bool Start() override;
	virtual void CloseService() override;
private:
	ListenerRef  _listener = nullptr;
};