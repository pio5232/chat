#pragma once
#include "NetworkBase.h"
#include "NetworkUtils.h"
#include <queue>

namespace C_Network
{
	class LanServer : public ServerBase
	{
	public:
		LanServer(const NetAddress& netAddr, uint maxSessionCnt, SessionCreator creator);
		~LanServer();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnError(int errCode, WCHAR* cause);

		// GameServer가 받게 될 방 번호를 저장 
	};
}