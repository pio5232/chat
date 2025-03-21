#pragma once

#include <functional>
#include <unordered_map>

namespace C_Network
{
	/*---------------------------------------
				BasePacketHandler						// 서버마다 상속받아서 사용하도록 한다..
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
		// 함수 정의
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
	//// 나중에 만들게 될 중계 서버, 중계 서버가 생기게 되면 패킷 처리에 대한 부분도 이 녀석이 1차로 검증을 한 후에 메인 로직 서버에 전달해야한다.
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
