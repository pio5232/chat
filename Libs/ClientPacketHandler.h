#pragma once

#include <functional>
#include "PacketDefine.h"
#include <unordered_map>
namespace C_Network
{
	/*---------------------------------------
				BasePacketHandler						// 서버마다 상속받아서 사용하도록 한다..
	---------------------------------------*/
	template <typename PacketHandlerType>
	class ClientPacketHandler
	{
	public:
		ClientPacketHandler() { _packetFuncsDic.clear(); }
		// uin16 Message크기는 제외한다. 어차피 PacketHandler가 처리하게 되는 것은 하나의 완전한 데이터로 왔을 때 처리하기 때문이다.
		//, uint16) > ; // sessionId, Message 내용 버퍼, Message 크기.
		using PacketFunc = ErrorCode(PacketHandlerType::*)(ULONGLONG, C_Utility::CSerializationBuffer&);//std::function<bool(ULONGLONG, char*)>;

		static SharedSendBuffer MakeSendBuffer(uint packetSize)
		{
			return std::make_shared<C_Utility::CSerializationBuffer>(packetSize);
		}

		// 직렬화 버퍼에 데이터를 채우자! 가변 템플릿을 활용. 길이가 고정되어 있는 경우는 값을 넣어놓고 이 함수를 통해 send.
		template <typename PacketType>
		static SharedSendBuffer MakePacket(PacketType& packet)// (uint16 packetType, PacketType& packet)
		{
			// TODO : CSerializationBuffer 또한 Pool에서 꺼내서 사용하도록 만든다.
			uint16 packetSize = sizeof(packet);

			SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(packetSize);

			*sendBuffer << packet;

			return sendBuffer;
		}

		SharedSendBuffer MakeErrorPacket(PacketErrorCode errorCode)
		{
			ErrorPacket errorPacket;

			errorPacket.packetErrorCode = errorCode;

			SharedSendBuffer sendBuffer = MakePacket(errorPacket);

			return sendBuffer;
		}
		
		//template <>
		//static SharedSendBuffer MakePacket<PacketHeader>(PacketHeader& packet)
		//{
		//	uint16 packetSize = sizeof(PacketHeader);

		//	SharedSendBuffer sendBuffer = std::make_shared < C_Utility::CSerializationBuffer>(packetSize);

		//	*sendBuffer << packet;

		//	return sendBuffer;
		//}

		ErrorCode ProcessPacket(ULONGLONG sessionId, uint16 packetType, C_Utility::CSerializationBuffer& buffer)
		{
			if (_packetFuncsDic.find(packetType) == _packetFuncsDic.end())
				return ErrorCode::CANNOT_FIND_PACKET_FUNC;

			return ((reinterpret_cast<PacketHandlerType*>(this))->*_packetFuncsDic[packetType])(sessionId, buffer);
		}
	protected:
		std::unordered_map<uint16, PacketFunc> _packetFuncsDic;

	};

	class ChattingClientPacketHandler : public ClientPacketHandler<ChattingClientPacketHandler>
	{
	public :
		ChattingClientPacketHandler(class ChattingServer* owner, class RoomManager* roomMgr, class UserManager* userMgr, class SessionManager* sessionMgr) : 
			_owner(owner), _roomMgr(roomMgr), _userMgr(userMgr), _sessionMgr(sessionMgr)
		{
			_packetFuncsDic[ROOM_LIST_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessRoomListRequestPacket;

			_packetFuncsDic[CHAT_TO_ROOM_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessChatToRoomPacket; // Chat To Room Users
			_packetFuncsDic[CHAT_TO_USER_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessChatToUserPacket; // Chat To Room Users

			_packetFuncsDic[LOG_IN_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessLogInPacket;
			_packetFuncsDic[MAKE_ROOM_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessMakeRoomRequestPacket;

			_packetFuncsDic[ENTER_ROOM_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessEnterRoomRequestPacket;
			_packetFuncsDic[LEAVE_ROOM_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessLeaveRoomRequestPacket;
		}
	private:
		// 함수 정의
		ErrorCode ProcessRoomListRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		
		ErrorCode ProcessLogInPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToRoomPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToUserPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessMakeRoomRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessEnterRoomRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessLeaveRoomRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);

		class ChattingServer* _owner;
		class RoomManager* _roomMgr;
		class UserManager* _userMgr;
		class SessionManager* _sessionMgr;
	};

	// 나중에 만들게 될 중계 서버, 중계 서버가 생기게 되면 패킷 처리에 대한 부분도 이 녀석이 1차로 검증을 한 후에 메인 로직 서버에 전달해야한다.
	class LogInClientPacketHandler : public ClientPacketHandler<LogInClientPacketHandler>
	{
	public:
		LogInClientPacketHandler(class LogInServer* owner) : _owner(owner)
		{
			_packetFuncsDic[LOG_IN_REQUEST_PACKET] = &LogInClientPacketHandler::ProcessLogInPacket;
		}
	private:
		ErrorCode ProcessLogInPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		class LogInServer* _owner;
	};

}
