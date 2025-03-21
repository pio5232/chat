#include "pch.h"
#include "LanServer.h"
#include "LanSession.h"
#include "PacketHandler.h"

C_Network::LanServer::LanServer(const NetAddress& netAddr, uint maxSessionCnt, SessionCreator creator) : ServerBase(netAddr, maxSessionCnt, creator)
{
	C_Network::LanClientPacketHandler::Init();
}

C_Network::LanServer::~LanServer()
{

}

bool C_Network::LanServer::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	return false;
}

void C_Network::LanServer::OnError(int errCode, WCHAR* cause)
{
}
