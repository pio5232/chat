#include "pch.h"
#include "LanSession.h"
#include "PacketHandler.h"
void C_Network::LanSession::OnConnected()
{
}

void C_Network::LanSession::OnDisconnected()
{

}

void C_Network::LanSession::OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type)
{
	LanSessionPtr sessionPtr = std::static_pointer_cast<C_Network::LanSession>(shared_from_this());

	if (ErrorCode::NONE != LanClientPacketHandler::ProcessPacket(sessionPtr, type, buffer))
	{
		// LOG
	}
}

