#pragma once

#include "NetworkBase.h"
#include "UserManager.h"
#include "ServerPacketHandler.h"

namespace C_Network
{
	class ChattingClient : public ClientBase
	{
	public:
		ChattingClient(UITaskManager* uiTaskManager, const NetAddress& targetEndPoint, uint maxSessionCnt = 1);

		//void RegisterPacketHandler(class ChattingServerPacketHandler* packetHandler)
		//{
		//	_packetHandler = packetHandler;
		//}
		virtual ~ChattingClient();
		virtual C_Network::NetworkErrorCode OnEnterServer() override;
		virtual C_Network::NetworkErrorCode OnLeave() override;
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer,  ULONGLONG sessionId,uint16 type) override; // 서버에서 온 메시지를 처리.
	private:
		std::unique_ptr<ChattingServerPacketHandler> _packetHandler;
	};
}
