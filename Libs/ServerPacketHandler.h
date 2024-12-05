#pragma once

#include <functional>
#include "PacketDefine.h"
#include <unordered_map>
#include "UITaskManager.h"

namespace C_Network
{
	// �������� �� ��Ŷ�� Ŭ���̾�Ʈ�� ó���Ѵ�.
	// -------------------------------------------------------
	//					BaseServerPacketHandler
	// -------------------------------------------------------

	template <typename PacketHandlerType>
	class ServerPacketHandler
	{
	public:
		// ������ Ŭ���̾�Ʈ�� ó���ϴ� �޽����� �������� �� �޽����̴�.
		//using PacketFunc = std::function<ErrorCode(C_Utility::CSerializationBuffer&)>;// ErrorCode(PacketHandlerType::*)(C_Utility::CSerializationBuffer&);
		using PacketFunc = ErrorCode(PacketHandlerType::*)(C_Utility::CSerializationBuffer&);

		ServerPacketHandler()
		{
			_packetFuncsDic.clear();
		}
		static SharedSendBuffer MakeSendBuffer(uint packetSize)
		{
			return std::make_shared<C_Utility::CSerializationBuffer>(packetSize);
		}

		// ����ȭ ���ۿ� �����͸� ä����! ���� ���ø��� Ȱ��. ���̰� �����Ǿ� �ִ� ���� ���� �־���� �� �Լ��� ���� send.
		//static SharedSendBuffer MakePacket(uint16 packetSize, PacketType& packet)// (uint16 packetType, PacketType& packet)
		template <typename PacketType>
		static SharedSendBuffer MakePacket(PacketType& packet)// (uint16 packetType, PacketType& packet)
		{
			// TODO : CSerializationBuffer ���� Pool���� ������ ����ϵ��� �����.
			uint16 packetSize = sizeof(packet);

			SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(packetSize);

			*sendBuffer << packet;

			return sendBuffer;
		}

		template <>
		static SharedSendBuffer MakePacket<PacketHeader>(PacketHeader& packet)
		{
			uint16 packetSize = sizeof(PacketHeader);

			SharedSendBuffer sendBuffer = std::make_shared < C_Utility::CSerializationBuffer>(packetSize);

			*sendBuffer << packet;

			return sendBuffer;
		}
		//// ����ȭ ���ۿ� �����͸� ä����! ���� ���ø��� Ȱ��.
		//template <typename PacketType>
		//static SharedSendBuffer MakePacket(uint16 packetType, PacketType& packet)//(uint16 packetType, uint16 packetSize, Types... args )
		//{
		//	// TODO : CSerializationBuffer ���� Pool���� ������ ����ϵ��� �����.

		//	SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(PACKET_SIZE_MAX);

		//	*sendBuffer << packet;

		//	return sendBuffer;
		//}

		ErrorCode ProcessPacket(ULONGLONG sessionId, uint16 packetType, C_Utility::CSerializationBuffer& buffer)
		{
			if (_packetFuncsDic.find(packetType) == _packetFuncsDic.end())
				return ErrorCode::CANNOT_FIND_PACKET_FUNC;

			//return ((reinterpret_cast<PacketHandlerType*>(this))->*_packetFuncsDic[packetType])(sessionId, buffer);
			return ((reinterpret_cast<PacketHandlerType*>(this))->*_packetFuncsDic[packetType])(buffer);
		}
	protected:
		std::unordered_map<uint16, PacketFunc> _packetFuncsDic;
	};

	//// �뷮 Ŭ���̾�Ʈ ���̸� �ڵ鸵�Ҷ��� ChattingServerPacketHandler�� ��ӹ޾Ƽ� ����ϵ��� �Ѵ�.
	class ChattingServerPacketHandler : public ServerPacketHandler<ChattingServerPacketHandler>
	{
	public:
		ChattingServerPacketHandler(UITaskManager* uiTaskManager, class ChattingClient* owner) : _uiTaskManager(uiTaskManager), _owner(owner)
		{
			Init();
		}

		void Init()
		{
			//type.. -> callback ���
			_packetFuncsDic[PacketType::ROOM_LIST_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessRoomListResponsePacket;
			_packetFuncsDic[PacketType::LOG_IN_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessLogInResponsePacket;
			_packetFuncsDic[PacketType::CHAT_TO_ROOM_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessChatToRoomResponsePacket;
			//_packetFuncsDic[PacketType::CHAT_NOTIFY_PACKET] = &ChattingServerPacketHandler::ProcessChatToRoomResponsePacket;
			_packetFuncsDic[PacketType::CHAT_TO_USER_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessChatToUserResponsePacket;

			_packetFuncsDic[PacketType::ENTER_ROOM_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessEnterRoomResponsePacket;
			//_packetFuncsDic[PacketType::ENTER_ROOM_NOTIFY_PACKET] = &ChattingServerPacketHandler::ProcessEnterRoomNotifyPacket; // �� 2���� ��ĥ��? ���

			_packetFuncsDic[PacketType::LEAVE_ROOM_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessChatToUserResponsePacket;
			//_packetFuncsDic[PacketType::LEAVE_ROOM_NOTIFY_PACKET] = &ChattingServerPacketHandler::ProcessLeaveRoomNotifyPacket;
		}
	private:

		// Ŭ���̾�Ʈ�� �̱� ������� �����ϱ� ������ ��� ������ �ʿ䰡 ���ݾ�!!
		// PacketHandler�� ������ �����͸� �.. UI�� �����ų ���·� ���� �Ŀ� . uiTaskManager�� ���� ui�� �����ϵ��� ��û�ϴ� ���̴�.

		ErrorCode ProcessRoomListResponsePacket(C_Utility::CSerializationBuffer& buffer);

		ErrorCode ProcessLogInResponsePacket(C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToRoomResponsePacket(C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToUserResponsePacket(C_Utility::CSerializationBuffer& buffer);

		ErrorCode ProcessEnterRoomResponsePacket(C_Utility::CSerializationBuffer& buffer);
		// ErrorCode ProcessEnterRoomNotifyPacket(C_Utility::CSerializationBuffer& buffer);  // ����?

		ErrorCode ProcessLeaveRoomResponsePacket(C_Utility::CSerializationBuffer& buffer);
		// ErrorCode ProcessLeaveRoomNotifyPacket(C_Utility::CSerializationBuffer& buffer);  // ����?

		UITaskManager* _uiTaskManager;
		class ChattingClient* _owner;
	};
}