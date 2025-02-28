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
