#include "pch.h"
#include "ServerPacketHandler.h"


PacketHandleFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	if (pkt.success() == false)
		return true;

	if (pkt.players().size() == 0)
	{
			//캐릭터가 없다.
	}

	//입장 UI버튼 눌러서 게임 입장
	Protocol::C_ENTER_GAME enterGamePkt;
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	//TODO 방에 입장을했으니 렌더링을 하든지 사람을 그려준다든지..
	return true;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	std::cout << pkt.msg() << endl;

	return true;
}


