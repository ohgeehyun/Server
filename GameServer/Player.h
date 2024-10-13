#pragma once
class Player
{
public:
	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::PlayerType_NONE;
	GameSessionRef			ownerSession;//Cycle
};

