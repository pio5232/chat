#pragma once

#include <functional>
#include <unordered_map>

namespace C_Network
{
	/*---------------------------------------
				BasePacketHandler						// �������� ��ӹ޾Ƽ� ����ϵ��� �Ѵ�..
	---------------------------------------*/
	
	class LobbyClientPacketHandler 
	{
		using PacketFunc = ErrorCode(*)(LobbySessionPtr&, C_Utility::CSerializationBuffer&);
	public:

		static void Init();

		static ErrorCode ProcessPacket(LobbySessionPtr& sharedSession, uint16 packetType, C_Utility::CSerializationBuffer& buffer)
		{
			if (_packetFuncsDic.find(packetType) == _packetFuncsDic.end())
				return ErrorCode::CANNOT_FIND_PACKET_FUNC;
			
			return _packetFuncsDic[packetType](sharedSession, buffer);

		}
	private:
		// �Լ� ����
		static ErrorCode ProcessRoomListRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);

		static ErrorCode ProcessLogInPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessChatToRoomPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessChatToUserPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessMakeRoomRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessEnterRoomRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessLeaveRoomRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessGameReadyRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);
		
		static ErrorCode ProcessHeartbeatPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer);

		static std::unordered_map<uint16, PacketFunc> _packetFuncsDic;
	};


	class LanClientPacketHandler
	{
	public:
		using PacketFunc = ErrorCode(*)(LanSessionPtr&, C_Utility::CSerializationBuffer&);

		static void Init();

		static ErrorCode ProcessPacket(LanSessionPtr& lanSessionPtr, uint16 packetType, C_Utility::CSerializationBuffer& buffer)
		{
			if (_packetFuncsDic.find(packetType) == _packetFuncsDic.end())
				return ErrorCode::CANNOT_FIND_PACKET_FUNC;

			return _packetFuncsDic[packetType](lanSessionPtr, buffer);

		}

		static ErrorCode ProcessLanInfoNotifyPacket(LanSessionPtr& lanSessionPtr, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessGameSettingRequestPacket(LanSessionPtr& lanSessionPtr, C_Utility::CSerializationBuffer& buffer);
		
	private:
		static std::unordered_map<uint16, PacketFunc> _packetFuncsDic;

	};
	//// ���߿� ����� �� �߰� ����, �߰� ������ ����� �Ǹ� ��Ŷ ó���� ���� �κе� �� �༮�� 1���� ������ �� �Ŀ� ���� ���� ������ �����ؾ��Ѵ�.
	//class LogInClientPacketHandler : public LobbyClientPacketHandler<LogInClientPacketHandler>
	//{
	//public:
	//	LogInClientPacketHandler(class LogInServer* owner) : _owner(owner)
	//	{
	//		_packetFuncsDic[LOG_IN_REQUEST_PACKET] = &LogInClientPacketHandler::ProcessLogInPacket;
	//	}
	//private:
	//	ErrorCode ProcessLogInPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer);
	//	class LogInServer* _owner;
	//};

}
