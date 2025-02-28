#include "pch.h"
#include "LobbySession.h"
#include "PacketHandler.h"
#include "Room.h"

void C_Network::LobbySession::OnConnected()
{
}

void C_Network::LobbySession::OnDisconnected()
{
	if (!_curRoom.expired())
	{
		RoomPtr roomPtr = _curRoom.lock();

		LobbySessionPtr lobbySessionPtr = std::static_pointer_cast<LobbySession>(shared_from_this());
		
		roomPtr->DoAsync(&Room::LeaveRoom, lobbySessionPtr);
	}
}

void C_Network::LobbySession::OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type)
{
	LobbySessionPtr sessionPtr = std::static_pointer_cast<LobbySession>(shared_from_this());

	if (ErrorCode::NONE != LobbyClientPacketHandler::ProcessPacket(sessionPtr, type, buffer))
	{
		// LOG
	}
}
