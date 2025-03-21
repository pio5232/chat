#include "pch.h"
#include "LobbyServer.h"
#include "LobbyMonitor.h"
#include "PacketHandler.h"
#include "LanServer.h"
#include "LanSession.h"

C_Network::LobbyServer::LobbyServer(const NetAddress& netAddr, uint maxSessionCnt, SessionCreator creator) : ServerBase(netAddr, maxSessionCnt, creator), _canCheckHeartbeat(true)
{
	C_Network::LobbyClientPacketHandler::Init();

	_monitor = std::make_unique<C_Utility::LobbyMonitor>(_sessionMgr.get());


	_lanServer = std::make_shared<LanServer>(NetAddress(netAddr.GetIpAddress(), LanServerPort), 10,
			[]() { return std::static_pointer_cast<Session>(std::make_shared<LanSession>()); });

	if (_lanServer == nullptr)
	{
		printf("LanServer is nullptr\n");
		//
		return;
	}

	_lanServer->Begin(true);

	_heartbeatCheckThread = std::thread([this]() {this->CheckHeartbeat(); });

}

C_Network::LobbyServer::~LobbyServer()
{
	_canCheckHeartbeat = false;

	if (_heartbeatCheckThread.joinable())
		_heartbeatCheckThread.join();

	_lanServer->End();
}

bool C_Network::LobbyServer::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	return false;
}

void C_Network::LobbyServer::OnError(int errCode, WCHAR* cause)
{
}

void C_Network::LobbyServer::CheckHeartbeat()
{
	printf("[ Check Heartbeat Start ]\n");
	while (_canCheckHeartbeat)
	{
		ULONGLONG now = C_Utility::GetTimeStamp();

		_sessionMgr->CheckHeartbeatTimeOut(now);

		Sleep(3000);
	}
	printf("[ Check Heartbeat End ]\n");
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
