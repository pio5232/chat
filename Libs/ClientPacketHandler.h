#pragma once

#include <functional>
#include "PacketDefine.h"
#include <unordered_map>
namespace C_Network
{
	/*---------------------------------------
				BasePacketHandler						// �������� ��ӹ޾Ƽ� ����ϵ��� �Ѵ�..
	---------------------------------------*/
	template <typename PacketHandlerType>
	class ClientPacketHandler
	{
	public:
		ClientPacketHandler() { _packetFuncsDic.clear(); }
		// uin16 Messageũ��� �����Ѵ�. ������ PacketHandler�� ó���ϰ� �Ǵ� ���� �ϳ��� ������ �����ͷ� ���� �� ó���ϱ� �����̴�.
		//, uint16) > ; // sessionId, Message ���� ����, Message ũ��.
		using PacketFunc = ErrorCode(PacketHandlerType::*)(ULONGLONG, C_Utility::CSerializationBuffer&);//std::function<bool(ULONGLONG, char*)>;

		static SharedSendBuffer MakeSendBuffer(uint packetSize)
		{
			return std::make_shared<C_Utility::CSerializationBuffer>(packetSize);
		}

		// ����ȭ ���ۿ� �����͸� ä����! ���� ���ø��� Ȱ��. ���̰� �����Ǿ� �ִ� ���� ���� �־���� �� �Լ��� ���� send.
		template <typename PacketType>
		static SharedSendBuffer MakePacket(uint16 packetSize, PacketType& packet)// (uint16 packetType, PacketType& packet)
		{
			// TODO : CSerializationBuffer ���� Pool���� ������ ����ϵ��� �����.

			SharedSendBuffer sendBuffer = std::make_shared<C_Utility::CSerializationBuffer>(packetSize);

			*sendBuffer << packet;

			return sendBuffer;
		}

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
		ChattingClientPacketHandler(class ChattingServer* owner, class RoomManager* roomMgr, class UserManager* userMgr) : _owner(owner), _roomMgr(roomMgr), _userMgr(userMgr)
		{
			_packetFuncsDic[ROOM_LIST_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessRoomListRequestPacket;

			_packetFuncsDic[CHAT_TO_ROOM_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessChatToRoomPacket; // Chat To Room Users
			_packetFuncsDic[CHAT_TO_USER_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessChatToUserPacket; // Chat To Room Users

			_packetFuncsDic[LOG_IN_REQUEST_PACKET] = &ChattingClientPacketHandler::ProcessLogInPacket;
		}
	private:
		// �Լ� ����
		ErrorCode ProcessRoomListRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		
		ErrorCode ProcessLogInPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToRoomPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
		ErrorCode ProcessChatToUserPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);

		class ChattingServer* _owner;
		class RoomManager* _roomMgr;
		class UserManager* _userMgr;
	};

	// ���߿� ����� �� �߰� ����, �߰� ������ ����� �Ǹ� ��Ŷ ó���� ���� �κе� �� �༮�� 1���� ������ �� �Ŀ� ���� ���� ������ �����ؾ��Ѵ�.
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