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

		// ����ȭ ���ۿ� �����͸� ä����! ���� ���ø��� Ȱ��. ���̰� �����Ǿ� �ִ� ���� ���� �־���� �� �Լ��� ���� send.
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
