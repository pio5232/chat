#pragma once

#include "NetworkBase.h"

namespace C_Network
{
	/*--------------------------
			EchoServer
	--------------------------*/
	// 현재 분기에서는 EchoClientPacketHandler가 정의되어 있지 않기 때문에 EchoServer를 사용하지 않는다.
	class EchoServer : public ServerBase
	{
	public:
		EchoServer(const NetAddress& netAddr, uint maxSessionCnt);
		~EchoServer();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId);
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type);
	private:
		// class EchoClientPacketHandler* _clientPacketHandler;

	};
}