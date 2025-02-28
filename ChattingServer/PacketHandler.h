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
		static ErrorCode ProcessRoomListRequestPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);

		static ErrorCode ProcessLogInPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessChatToRoomPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessChatToUserPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessMakeRoomRequestPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessEnterRoomRequestPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessLeaveRoomRequestPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);
		static ErrorCode ProcessGameReadyRequestPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer);
		

		static std::unordered_map<uint16, PacketFunc> _packetFuncsDic;
	};


	//class LanClientPacketHandler : public LobbyClientPacketHandler<LanClientPacketHandler>
	//{
	//public:
	//	LanClientPacketHandler(class SessionManager* sessionMgr, class LanServer* owner) : _sessionMgr(sessionMgr)
	//		, _owner(owner)
	//	{
	//		_packetFuncsDic[GAME_SERVER_INFO_NOTIFY_PACKET] = &LanClientPacketHandler::ProcessLanInfoNotifyPacket;
	//	}

	//	ErrorCode ProcessLanInfoNotifyPacket(SessionPtr& sharedSession,, C_Utility::CSerializationBuffer& buffer);
	//private:
	//	class SessionManager* _sessionMgr;
	//	class LanServer* _owner;
	//};
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
