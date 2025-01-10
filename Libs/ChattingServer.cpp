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

	//_monitor = std::make_unique<C_Utility::ChatMonitor>(_roomMgr.get(), _userMgr.get());
			// TODO : USE POOL, 스마트 포인터에 대해서도 pool을 적용할 수 있다면 좋을 것임.
	_packetHandler = new ChattingClientPacketHandler(this, _roomMgr.get(), _userMgr.get(), _sessionMgr.get());
}

C_Network::ChattingServer::~ChattingServer()
{
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
	_userMgr->DeleteUser(sessionId);
	// TODO : 해당 세션의 user Id 를 찾아서 데이터 저장 및 제거 필요.
}

void C_Network::ChattingServer::OnError(int errCode, WCHAR* cause)
{
}

void C_Network::ChattingServer::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
	if (_packetHandler->ProcessPacket(sessionId, type, buffer) == ErrorCode::CANNOT_FIND_PACKET_FUNC)
		TODO_LOG;
}

C_Network::NetworkErrorCode C_Network::ChattingServer::SendToAllUser(C_Network::SharedSendBuffer& buffer)
{
	return NetworkErrorCode();
}

C_Network::NetworkErrorCode C_Network::ChattingServer::SendToRoom(C_Network::SharedSendBuffer& buffer, uint16 roomNum)
{
	return NetworkErrorCode();
}
