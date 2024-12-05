#pragma once

#include <functional>
#include "PacketDefine.h"
#include <unordered_map>
#include "UITaskManager.h"

namespace C_Network
{
	// 서버에서 온 패킷을 클라이언트가 처리한다.
	// -------------------------------------------------------
	//					BaseServerPacketHandler
	// -------------------------------------------------------

	template <typename PacketHandlerType>
	class ServerPacketHandler
	{
	public:
		// 어차피 클라이언트가 처리하는 메시지는 서버에서 온 메시지이다.
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

		// 직렬화 버퍼에 데이터를 채우자! 가변 템플릿을 활용. 길이가 고정되어 있는 경우는 값을 넣어놓고 이 함수를 통해 send.
		//static SharedSendBuffer MakePacket(uint16 packetSize, PacketType& packet)// (uint16 packetType, PacketType& packet)
		template <typename PacketType>
		static SharedSendBuffer MakePacket(PacketType& packet)// (uint16 packetType, PacketType& packet)
		{
			// TODO : CSerializationBuffer 또한 Pool에서 꺼내서 사용하도록 만든다.
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
		//// 직렬화 버퍼에 데이터를 채우자! 가변 템플릿을 활용.
		//template <typename PacketType>
		//static SharedSendBuffer MakePacket(uint16 packetType, PacketType& packet)//(uint16 packetType, uint16 packetSize, Types... args )
		//{
		//	// TODO : CSerializationBuffer 또한 Pool에서 꺼내서 사용하도록 만든다.

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

	//// 대량 클라이언트 더미를 핸들링할때는 ChattingServerPacketHandler를 상속받아서 사용하도록 한다.
	class ChattingServerPacketHandler : public ServerPacketHandler<ChattingServerPacketHandler>
	{
	public:
		ChattingServerPacketHandler(UITaskManager* uiTaskManager, class ChattingClient* owner) : _uiTaskManager(uiTaskManager), _owner(owner)
		{
			Init();
		}

		void Init()
		{
			//type.. -> callback 등록
			_packetFuncsDic[PacketType::ROOM_LIST_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessRoomListResponsePacket;
			_packetFuncsDic[PacketType::LOG_IN_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessLogInResponsePacket;
			_packetFuncsDic[PacketType::CHAT_TO_ROOM_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessChatToRoomResponsePacket;
			//_packetFuncsDic[PacketType::CHAT_NOTIFY_PACKET] = &ChattingServerPacketHandler::ProcessChatToRoomResponsePacket;
			_packetFuncsDic[PacketType::CHAT_TO_USER_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessChatToUserResponsePacket;

			_packetFuncsDic[PacketType::ENTER_ROOM_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessEnterRoomResponsePacket;
			//_packetFuncsDic[PacketType::ENTER_ROOM_NOTIFY_PACKET] = &ChattingServerPacketHandler::ProcessEnterRoomNotifyPacket; // 위 2개를 합칠까? 고민

			_packetFuncsDic[PacketType::LEAVE_ROOM_RESPONSE_PACKET] = &ChattingServerPacketHandler::ProcessChatToUserResponsePacket;
			//_packetFuncsDic[PacketType::LEAVE_ROOM_NOTIFY_PACKET] = &ChattingServerPacketHandler::ProcessLeaveRoomNotifyPacket;
		}
	private:

		// 클라이언트는 싱글 스레드로 동작하기 때문에 사실 걱정할 필요가 없잖아!!
		// PacketHandler의 역할은 데이터를 까서.. UI에 적용시킬 형태로 만든 후에 . uiTaskManager를 통해 ui를 변경하도록 요청하는 것이다.

		ErrorCode ProcessRoomListResponsePacket(C_Utility::CSerializationBuffer& buffer);

		ErrorCode ProcessLogInResponsePacket(C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToRoomResponsePacket(C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToUserResponsePacket(C_Utility::CSerializationBuffer& buffer);

		ErrorCode ProcessEnterRoomResponsePacket(C_Utility::CSerializationBuffer& buffer);
		// ErrorCode ProcessEnterRoomNotifyPacket(C_Utility::CSerializationBuffer& buffer);  // 합쳐?

		ErrorCode ProcessLeaveRoomResponsePacket(C_Utility::CSerializationBuffer& buffer);
		// ErrorCode ProcessLeaveRoomNotifyPacket(C_Utility::CSerializationBuffer& buffer);  // 합쳐?

		UITaskManager* _uiTaskManager;
		class ChattingClient* _owner;
	};
}