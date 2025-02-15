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
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId);
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type);

		NetworkErrorCode SendToAllUser(SharedSendBuffer buffer, NetCallbackType callbackType);
		NetworkErrorCode SendToRoom(SharedSendBuffer buffer, uint16 roomNum, NetCallbackType callbackType);

	private:
		class ChattingClientPacketHandler* _packetHandler;
		std::unique_ptr<RoomManager> _roomMgr;
		std::unique_ptr<UserManager> _userMgr;
		std::unique_ptr<LanServer> _lanServer;

	};
}