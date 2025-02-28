#pragma once

#include "NetworkBase.h"
#include "NetworkUtils.h"

namespace C_Network
{
	class LobbyServer : public ServerBase
	{
	public:
		LobbyServer(const NetAddress& netAddr, uint maxSessionCnt, SessionCreator creator);
		~LobbyServer();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnError(int errCode, WCHAR* cause);

#ifdef LAN
		std::unique_ptr<LanServer> _lanServer;
#endif // LAN

	};
}