#pragma once
#include "PacketDefine.h"

namespace C_Network
{
	class PacketMaker
	{
	public:
		static SharedSendBuffer MakeSendBuffer(uint packetSize)
		{
			return std::make_shared<C_Utility::CSerializationBuffer>(packetSize);
		}

		// 직렬화 버퍼에 데이터를 채우자! 가변 템플릿을 활용. 길이가 고정되어 있는 경우는 값을 넣어놓고 이 함수를 통해 send.
		template <typename PacketType>
		static SharedSendBuffer MakePacket(PacketType& packet)// (uint16 packetType, PacketType& packet)
		{
			// AA
			uint16 packetSize = sizeof(packet);

			SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(packetSize);

			*sendBuffer << packet;

			return sendBuffer;
		}

		static SharedSendBuffer MakeErrorPacket(PacketErrorCode errorCode)
		{
			ErrorPacket errorPacket;

			errorPacket.packetErrorCode = errorCode;

			SharedSendBuffer sendBuffer = MakePacket(errorPacket);

			return sendBuffer;
		}
	};
}
