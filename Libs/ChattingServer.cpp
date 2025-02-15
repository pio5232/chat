#include "LibsPch.h"
#include "ChattingServer.h"
#include "ClientPacketHandler.h"

C_Network::ChattingServer::ChattingServer(const NetAddress& netAddr, uint maxSessionCnt) : ServerBase(netAddr, maxSessionCnt)
{
	const uint roomCnt = 20;
	
	uint maxRoomUserCnt = maxSessionCnt / roomCnt;
	if (maxSessionCnt % roomCnt != 0)
		++maxRoomUserCnt;
	
	_userMgr = std::make_unique<UserManager>(maxSessionCnt);
	_roomMgr = std::make_unique<RoomManager>(this, roomCnt, maxRoomUserCnt,_userMgr.get());

	_monitor = std::make_unique<C_Utility::ChatMonitor>(_roomMgr.get(), _userMgr.get());

	// AA
	_packetHandler = new ChattingClientPacketHandler(this, _roomMgr.get(), _userMgr.get(), _sessionMgr.get());

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

}

C_Network::ChattingServer::~ChattingServer()
{
	_lanServer->End();
	delete _packetHandler;
}

bool C_Network::ChattingServer::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	return false;
}

void C_Network::ChattingServer::OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId)
{
}

void C_Network::ChattingServer::OnDisconnected(ULONGLONG sessionId)
{
	SharedUser sharedUser = _userMgr->GetUserBySessionId(sessionId);

	if (nullptr == sharedUser)
	{
		printf("OnDisconnected - sharedUser is Null\n");
		return;
	}

	std::weak_ptr<Room> connectedRoom = sharedUser->GetConnectedRoom();
	if (!connectedRoom.expired())
	{
		SharedRoom sharedRoom = connectedRoom.lock();

		sharedRoom->DoAsync(&Room::LeaveRoom, sharedUser->GetUserId());
	}

	_userMgr->DeleteUser(sharedUser->GetUserId());
}

void C_Network::ChattingServer::OnError(int errCode, WCHAR* cause)
{
}

void C_Network::ChattingServer::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
	if (_packetHandler->ProcessPacket(sessionId, type, buffer) == ErrorCode::CANNOT_FIND_PACKET_FUNC)
	{

	}
}


C_Network::NetworkErrorCode C_Network::ChattingServer::SendToAllUser(C_Network::SharedSendBuffer buffer, NetCallbackType callbackType)
{
	return NetworkErrorCode();
}

C_Network::NetworkErrorCode C_Network::ChattingServer::SendToRoom(C_Network::SharedSendBuffer buffer, uint16 roomNum, NetCallbackType callbackType)
{
	if (callbackType == NetCallbackType::SEND_IPENDPOINT)
	{
		SharedRoom sharedRoom = _roomMgr->GetRoom(roomNum);
		
		sharedRoom->DoAsync(&Room::SendToAll, buffer, 0UI64, false);
	}

	return NetworkErrorCode::NONE;
}
