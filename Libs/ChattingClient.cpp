#include "LibsPch.h"
#include "ChattingClient.h"
#include "ServerPacketHandler.h"

C_Network::ChattingClient::ChattingClient(UITaskManager* uiTaskManager, const NetAddress& targetEndPoint, uint maxSessionCnt) : ClientBase(targetEndPoint, maxSessionCnt)
{
	_packetHandler = std::make_unique<ChattingServerPacketHandler>(uiTaskManager, this);
}


C_Network::ChattingClient::~ChattingClient()
{
}

C_Network::NetworkErrorCode C_Network::ChattingClient::OnEnterServer()
{
	TODO_LOG; // ENTER SERVER LOG
	//LogInRequestPacket

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ChattingClient::OnLeave()
{
	return C_Network::NetworkErrorCode::NONE;
}

void C_Network::ChattingClient::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
	if (_packetHandler->ProcessPacket(sessionId, type, buffer) == ErrorCode::CANNOT_FIND_PACKET_FUNC)
		TODO_LOG;
}
