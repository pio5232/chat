#include "LibsPch.h"
#include "ChattingServer.h"
#include "ClientPacketHandler.h"

C_Network::ChattingServer::ChattingServer(const NetAddress& netAddr, uint maxSessionCnt) : ServerBase(netAddr, maxSessionCnt)
{

	const uint roomCnt = 20;
	
	uint maxRoomUserCnt = maxSessionCnt / roomCnt;
	if (maxSessionCnt % roomCnt != 0)
		++maxRoomUserCnt;
	
	_roomMgr = std::make_unique<RoomManager>(this, roomCnt, maxRoomUserCnt);
	_userMgr = std::make_unique<UserManager>(maxSessionCnt);
	
	// TODO : USE POOL, ����Ʈ �����Ϳ� ���ؼ��� pool�� ������ �� �ִٸ� ���� ����.
	_packetHandler = new ChattingClientPacketHandler(this, _roomMgr.get(), _userMgr.get());
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
	// TODO : �ش� ������ user Id �� ã�Ƽ� ������ ���� �� ���� �ʿ�.
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
