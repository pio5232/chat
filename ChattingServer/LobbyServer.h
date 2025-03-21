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

	private:

		void CheckHeartbeat();
		volatile bool _canCheckHeartbeat;

		std::shared_ptr<class LanServer> _lanServer;
		std::thread _heartbeatCheckThread;
	};
}