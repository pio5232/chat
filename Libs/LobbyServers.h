#pragma once

#include "NetworkBase.h"
#include "NetworkUtils.h"
#include "UserManager.h"

namespace C_Network
{
	class ChattingServer : public ServerBase
	{
	public:
		ChattingServer(const NetAddress& netAddr, uint maxSessionCnt);
		~ChattingServer();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnError(int errCode, WCHAR* cause);

	private:
		std::unique_ptr<RoomManager> _roomMgr;
		std::unique_ptr<UserManager> _userMgr;
#ifdef LAN
		std::unique_ptr<LanServer> _lanServer;
#endif // LAN

	};
}