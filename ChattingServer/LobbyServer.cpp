#include "pch.h"
#include "LobbyServer.h"
#include "LobbyMonitor.h"
#include "PacketHandler.h"
C_Network::LobbyServer::LobbyServer(const NetAddress& netAddr, uint maxSessionCnt, SessionCreator creator) : ServerBase(netAddr, maxSessionCnt, creator)
{
	const uint roomCnt = 20;

	uint maxRoomUserCnt = maxSessionCnt / roomCnt;
	if (maxSessionCnt % roomCnt != 0)
		++maxRoomUserCnt;

	C_Network::LobbyClientPacketHandler::Init();

	_monitor = std::make_unique<C_Utility::LobbyMonitor>(_sessionMgr.get());


#ifdef LAN


	_lanServer = std::make_unique<C_Network::LanServer>(NetAddress(netAddr.GetIpAddress(), LanServerPort), 10);

	if (_userMgr == nullptr || _roomMgr == nullptr || _monitor == nullptr || _lanServer == nullptr)
	{
		printf("ChattingServer Initializing Failed\n");
		return;
	}

	_lanServer->RegistCallback([this](C_Network::SharedSendBuffer buffer, uint16 roomNum, NetCallbackType callbackType)
		{
			if (roomNum == 0)
			{
				SendToAllUser(buffer, callbackType);
			}
			else
			{
				SendToRoom(buffer, roomNum, callbackType);
			}
		});
	_lanServer->Begin(true);
#endif //  

}

C_Network::LobbyServer::~LobbyServer()
{
#ifdef LAN
	_lanServer->End();
#endif // LAN

}

bool C_Network::LobbyServer::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	return false;
}

void C_Network::LobbyServer::OnError(int errCode, WCHAR* cause)
{
}

//C_Network::NetworkErrorCode C_Network::LobbyServer::SendToRoom(C_Network::SharedSendBuffer buffer, uint16 roomNum, NetCallbackType callbackType)
//{
//	if (callbackType == NetCallbackType::SEND_IPENDPOINT)
//	{
//		RoomPtr sharedRoom = _roomMgr->GetRoom(roomNum);
//
//		sharedRoom->DoAsync(&Room::SendToAll, buffer, 0UI64, false);
//	}
//
//	return NetworkErrorCode::NONE;
//}
