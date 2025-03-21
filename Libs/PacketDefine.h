#pragma once
using serializationBuffer = C_Utility::CSerializationBuffer;

namespace C_Network
{
	const int ServerPort = 6000;
	const int LanServerPort = 8768;

	const int MAX_PACKET_SIZE = 1000;
	const ULONGLONG HEARTBEAT_TIMEOUT = 30000; // MS, 30��

	// AAA �����̳� ������ ������ ��Ŷ���� ��ġ����?. (EX) EnterRoom / LeaveRoom Request Packet�� ���� ��Ұ� �Ȱ��� ������ �ϳ��� ��ġ�� �÷��׸� �����ϴ� ������ ����, (EnterRoom Notify / LeaveRoomNotify)

	/// <summary>
	/// ���Ǽ��� ���ؼ� ����ü�� �����ϰ� ������, ������ �Ǹ� ����ü�� �����ص���..
	/// </summary>
	enum PacketType : uint16 // packet order
	{
		INVALID_PACKET = 65535,
		// C_S, REQUEST = Ŭ���̾�Ʈ -> ����
		// S_C, RESPONSE = ���� -> Ŭ���̾�Ʈ
		LOG_IN_REQUEST_PACKET = 0, //
		LOG_IN_RESPONSE_PACKET,

		ROOM_LIST_REQUEST_PACKET,
		ROOM_LIST_RESPONSE_PACKET,

		MAKE_ROOM_REQUEST_PACKET,
		MAKE_ROOM_RESPONSE_PACKET, // -> ENTER_ROOM_RESPONSE_PACKET���� RESPONSE�� ����.

		ENTER_ROOM_REQUEST_PACKET,
		ENTER_ROOM_RESPONSE_PACKET,
		ENTER_ROOM_NOTIFY_PACKET,

		LEAVE_ROOM_REQUEST_PACKET,
		LEAVE_ROOM_RESPONSE_PACKET,
		LEAVE_ROOM_NOTIFY_PACKET,

		CLIENT_LOG_OUT_PACKET,
		CLIENT_LOG_OUT_NOTIFY_PACKET,

		OWNER_CHANGE_NOTIFY_PACKET,
		// Ŭ���̾�Ʈ�� ��û
		CHAT_TO_ROOM_REQUEST_PACKET, // �� ���� �������� �޽��� ���� ��û, RoomNum�� -1�̸� ��� �濡 ����.
		CHAT_TO_USER_REQUEST_PACKET, // �ϳ��� �������� �޽��� ���� ��û.
		CHAT_NOTIFY_PACKET,

		// ������ ����.
		CHAT_TO_ROOM_RESPONSE_PACKET,
		CHAT_TO_USER_RESPONSE_PACKET, // 

		GAME_READY_REQUEST_PACKET,
		//GAME_READY_RESPONSE_PACKET,
		GAME_READY_NOTIFY_PACKET,
		GAME_START_NOTIFY_PACKET,

		ERROR_PACKET, // Ŭ���̾�Ʈ���� ������ �������� ��ȿ���� ���� ��û�� �Ǿ �������� ������ �Բ� �����ϵ��� �Ѵ�. ex) ������ �濡 ������ �ϴ� ����

		// -- LAN
		GAME_SERVER_SETTING_REQUEST_PACKET, // 1. ó�� GAME LOBBY�� �����ϸ� ������ ��û��Ŷ.(GAME -> LOBBY)
		GAME_SERVER_SETTING_RESPONSE_PACKET, // 2. ���� ������ ������ �� �������� ���� (LOBBY -> GAME)
		GAME_SERVER_LAN_INFO_PACKET, // 3. �ڽ� ip,port �� ���� ���� (GAME -> LOBBY) �̰� ������ �濡 �ִ� ���̵鿡�� �ٷ� ����.

		// -- GAME
		ENTER_GAME_REQUEST_PACKET,
		ENTER_GAME_RESPONSE_PACKET,
		LEAVE_GAME_NOTIFY_PACKET,

		MAKE_MY_CHARACTER_PACKET,
		MAKE_OTHER_CHARACTER_PACKET,

		GAME_LOAD_COMPELTE_PACKET, // MAKE PLAYER �� �� �۾��� �Ǿ��ٸ�.

		MOVE_START_REQUEST_PACKET,
		MOVE_START_NOTIFY_PACKET, // RESPONSE �� NOTIFY. ��� �������� ����
		MOVE_STOP_REQUEST_PACKET,
		MOVE_STOP_NOTIFY_PACKET, // RESPONSE �� NOTIFY. ��� �������� ����.

		CHARACTER_POSITION_SYNC_PACKET,
		UPDATE_POSITION_PACKET,

		HEART_BEAT_PACKET, // ���� ������ ���� ��Ŷ, 5�ʿ� �ϳ��� �������� �Ѵ�. // 30�ʰ� ������ ������ ���� �ɷ� üũ.

		ECHO_PACKET = 65534,
	};

	enum PacketErrorCode : uint16
	{
		REQUEST_DESTROYED_ROOM = 0, // �̹� ������ �濡 ���� ��û
		REQUEST_DIFF_ROOM_NAME, // ��û�� �� ������ �ٸ���.

		FULL_ROOM, // �ο��� �� á��.
		ALREADY_RUNNING_ROOM, // �̹� ������ ���۵� ���̴�.

		// -- GAME
		CONNECTED_FAILED_WRONG_TOKEN, // ���� ������ ������ �� ��ū�� �߸��� ��ū�̴�.
	};
	// �׻� padding�� �����ϴ��� Ȯ���ؾ��Ѵ�.
	enum PacketSize
	{
		PACKET_SIZE_ECHO = 10,

		PACKET_SIZE_MAX = 500,
	};

	// size = payload�� size
	// ��Ʈ��ũ �ڵ忡�� PacketHeader�� �и��ϰ� �Ǹ� PacketHeader��ŭ�� ������� ������� �ʴ´�.
	struct PacketHeader
	{
	public:
		//void SetSize(uint16 headerSize) { size = headerSize; }
		uint16 size = 0;
		uint16 type = INVALID_PACKET; 
	};

	// ECHO REQUEST / RESPONSE
	//struct EchoPacket : public PacketHeader
	//{
	//public:
	//	EchoPacket() : data(0) { size = sizeof(__int64), type = ECHO_PACKET; }
	//	__int64 data;
	//};

	struct ErrorPacket : public PacketHeader
	{
		ErrorPacket() { size = sizeof(packetErrorCode); type = ERROR_PACKET; }
		uint16 packetErrorCode = 0;
	};
	// CHAT 

	struct ChatUserRequestPacket : public PacketHeader
	{
	public:
		//ULONGLONG sendUserId = 0;
		ULONGLONG targetUserId = 0;
		uint16 messageLen = 0;
		WCHAR payLoad[0];		//WCHAR payLoad[MESSAGE_MAX_LEN];

	};
	struct ChatRoomRequestPacket : public PacketHeader
	{
		// AA
	public:
		uint16 roomNum = UINT16_MAX;
		uint16 messageLen = 0;
		WCHAR payLoad[0];
	};
	
	// head�� ����.
	struct ChatUserResponsePacket : public PacketHeader
	{
		ChatUserResponsePacket(){ type = CHAT_TO_USER_RESPONSE_PACKET; }
	};
	
	struct ChatRoomResponsePacket : public PacketHeader 
	{
		ChatRoomResponsePacket() { type = CHAT_TO_ROOM_RESPONSE_PACKET; }
	};

	struct LeaveRoomResponsePacket : public PacketHeader
	{
		LeaveRoomResponsePacket() { type = LEAVE_ROOM_RESPONSE_PACKET; }
	};





	struct ChatOtherUserNotifyPacket : public PacketHeader
	{
	public :
		ChatOtherUserNotifyPacket() { type = CHAT_NOTIFY_PACKET; }
		ULONGLONG sendUserId = 0;
		uint16 messageLen = 0;
		WCHAR payLoad[0];
	};
	// LOG_IN
	// ��ȣȭ.. ��ȣȭ?
	struct alignas (32) LogInRequestPacket : public PacketHeader
	{
	public:
		LogInRequestPacket() { size = sizeof(logInId) + sizeof(logInPw); type = LOG_IN_REQUEST_PACKET; }
		ULONGLONG logInId = 0;
		ULONGLONG logInPw = 0;
	};

	struct alignas (16) LogInResponsePacket : public PacketHeader
	{
		LogInResponsePacket() { size = sizeof(userId); type = LOG_IN_RESPONSE_PACKET; }
		ULONGLONG userId = 0;
	};

	// LOG_OUT
	struct LogOutRequestPacket : public PacketHeader
	{

	};
	struct LogOutResponsePacket : public PacketHeader
	{

	};

	// OWNER CHANGE NOTIFY
	struct OwnerChangeNotifyPacket : PacketHeader
	{
		OwnerChangeNotifyPacket() { size = sizeof(userId); type = OWNER_CHANGE_NOTIFY_PACKET; }
		ULONGLONG userId = 0;
	};

	struct LogOutNotifyPacket : PacketHeader
	{
		LogOutNotifyPacket() { size = sizeof(userId); type = CLIENT_LOG_OUT_NOTIFY_PACKET; }
		ULONGLONG userId = 0;
	};

	struct MakeRoomRequestPacket : public PacketHeader
	{
		MakeRoomRequestPacket() { size = sizeof(roomName); type = MAKE_ROOM_REQUEST_PACKET; }
		WCHAR roomName[ROOM_NAME_MAX_LEN]{};
	};

	struct MakeRoomResponsePacket : public PacketHeader
	{
		MakeRoomResponsePacket() { size = sizeof(isMade) + RoomInfo::GetSize();  type = MAKE_ROOM_RESPONSE_PACKET; }
		bool isMade = false;
		RoomInfo roomInfo{};
	};

	//struct alignas (64) EnterRoomResponsePacket : public PacketHeader
	struct EnterRoomResponsePacket : public PacketHeader
	{
		EnterRoomResponsePacket() {  type = ENTER_ROOM_RESPONSE_PACKET; } 
		bool bAllow = false;
		uint16 idCnt = 0;
		ULONGLONG ids[0]; // ������ id �������� �ֻ��� bit�� Ready ���¸� �������� �Ѵ�.

		//RoomInfo roomInfo = {};
	};

	// ENTER_ROOM
	struct EnterRoomRequestPacket : public PacketHeader
	{
		EnterRoomRequestPacket() { size = sizeof(roomNum) + sizeof(roomName); type = ENTER_ROOM_REQUEST_PACKET; }
		uint16 roomNum = 0;
		WCHAR roomName[ROOM_NAME_MAX_LEN]{};
	};

	// other notify
	struct EnterRoomNotifyPacket : public PacketHeader
	{
		EnterRoomNotifyPacket() { size = sizeof(enterUserId); type = ENTER_ROOM_NOTIFY_PACKET; }
		ULONGLONG enterUserId = 0;
	};

	struct LeaveRoomNotifyPacket : public PacketHeader
	{
		LeaveRoomNotifyPacket() { size = sizeof(leaveUserId); type = LEAVE_ROOM_NOTIFY_PACKET; }
		ULONGLONG leaveUserId = 0;
	};

	// LEAVE_ROOM
	struct LeaveRoomRequestPacket : public PacketHeader
	{
		LeaveRoomRequestPacket() { size = sizeof(roomNum) + sizeof(roomName); type = LEAVE_ROOM_REQUEST_PACKET; }
		uint16 roomNum = 0;
		WCHAR roomName[ROOM_NAME_MAX_LEN]{};
	};

	// REQUEST ROOM LIST
	struct RoomListRequestPacket : public PacketHeader
	{
	public:
		RoomListRequestPacket() { type = ROOM_LIST_REQUEST_PACKET; }
	};
	struct RoomListResponsePacket : public PacketHeader
	{
		RoomListResponsePacket() { type = ROOM_LIST_RESPONSE_PACKET; }
		uint16 roomCnt = 0;
		RoomInfo roomInfos[0];
	};	

	// GameReadyPacket
	struct GameReadyRequestPacket : public PacketHeader
	{
	public:
		GameReadyRequestPacket() { type = GAME_READY_REQUEST_PACKET; size = sizeof(isReady); }
		bool isReady = false;
	};

	struct GameReadyNotifyPacket : public PacketHeader
	{
	public:
		GameReadyNotifyPacket() { type = GAME_READY_NOTIFY_PACKET; size = sizeof(isReady) + sizeof(userId); }
		bool isReady = false;
		ULONGLONG userId = 0;
	};


	struct GameStartNotifyPacket : public PacketHeader
	{
		GameStartNotifyPacket() { type = GAME_START_NOTIFY_PACKET;}
	};
}

// ---- LAN
namespace C_Network
{
	struct GameServerSettingRequestPacket :public PacketHeader
	{
		GameServerSettingRequestPacket() { type = GAME_SERVER_SETTING_REQUEST_PACKET; }
	};

	struct GameServerSettingResponsePacket :public PacketHeader
	{
		GameServerSettingResponsePacket() { type = GAME_SERVER_SETTING_RESPONSE_PACKET; size = sizeof(roomNum) + sizeof(requiredUsers) + sizeof(maxUsers); }

		uint16 roomNum = 0;
		uint16 requiredUsers = 0; // �ε� ���� ��
		uint16 maxUsers = 0; // Max, Max - required�� AI�� ����� ��.
	};

	struct GameServerLanInfoPacket :public PacketHeader
	{
		GameServerLanInfoPacket() { type = GAME_SERVER_LAN_INFO_PACKET; size = sizeof(ipStr) + sizeof(port) + sizeof(roomNum) + sizeof(xorToken); }

		WCHAR ipStr[IP_STRING_LEN] = {};
		uint16 port = 0;
		uint16 roomNum = 0;
		ULONGLONG xorToken = 0;
	};


}

// ------ GAME
namespace C_Network
{
	struct EnterGameRequestPacket :public PacketHeader
	{
		EnterGameRequestPacket() { type = ENTER_GAME_REQUEST_PACKET; size = sizeof(userId) + sizeof(token); }

		ULONGLONG userId = 0;
		ULONGLONG token = 0;
	};

	struct EnterGameResponsePacket : public PacketHeader
	{
		EnterGameResponsePacket() { type = ENTER_GAME_RESPONSE_PACKET; }
	};

	struct LeaveGameNotifyPacket : public PacketHeader
	{
		LeaveGameNotifyPacket() { type = LEAVE_GAME_NOTIFY_PACKET; size = sizeof(entityId); }
		ULONGLONG entityId = 0;
	};

	struct MakeMyCharacterPacket : public PacketHeader
	{
		MakeMyCharacterPacket() { type = MAKE_MY_CHARACTER_PACKET; size = sizeof(entityId) + sizeof(pos); }
		ULONGLONG entityId = 0;
		Vector3 pos{};
	};

	struct MakeOtherCharacterPacket : public PacketHeader
	{
		MakeOtherCharacterPacket() { type = MAKE_OTHER_CHARACTER_PACKET; size = sizeof(entityId) + sizeof(pos) + sizeof(isAi); }
		ULONGLONG entityId = 0;
		Vector3 pos{};
		bool isAi = true;
	};

	struct GameLoadCompletePacket : public PacketHeader
	{
		GameLoadCompletePacket() { type = GAME_LOAD_COMPELTE_PACKET; }
	};

	// ���� GameStartPacket�� ���ư�.

	enum MoveDir : uint16
	{
		LEFT = 0,
		LEFT_UP,
		UP,
		RIGHT_UP,
		RIGHT,
		RIGHT_DOWN,
		DOWN,
		LEFT_DOWN,

		DIR_MAX,

		STOP = 65535,
	};
	struct MoveStartRequestPacket : public PacketHeader
	{
		MoveStartRequestPacket() { type = MOVE_START_REQUEST_PACKET; size = sizeof(pos) + sizeof(moveDIr); }
		Vector3 pos{};
		uint16 moveDIr = DIR_MAX;

	};

	struct MoveStartNotifyPacket : public PacketHeader
	{
		MoveStartNotifyPacket() { type = MOVE_START_NOTIFY_PACKET; size = sizeof(entityId) + sizeof(pos) + sizeof(moveDir); }
		ULONGLONG entityId = 0;
		Vector3 pos{};
		uint16 moveDir = DIR_MAX;
	};

	struct MoveStopRequestPacket : public PacketHeader
	{
		MoveStopRequestPacket() { type = MOVE_STOP_REQUEST_PACKET; size = sizeof(stopPos) + sizeof(stopDir); }
		Vector3 stopPos{};
		uint16 stopDir = DIR_MAX;
	};

	struct MoveStopNotifyPacket : public PacketHeader
	{
		MoveStopNotifyPacket() { type = MOVE_STOP_NOTIFY_PACKET; size = sizeof(entityId) + sizeof(stopPos) + sizeof(stopDir); }
		ULONGLONG entityId = 0;
		Vector3 stopPos{};
		uint16 stopDir = DIR_MAX;
	};

	struct CharacterPositionSyncPacket : public PacketHeader
	{
		CharacterPositionSyncPacket() { type = CHARACTER_POSITION_SYNC_PACKET; size = sizeof(entityId) + sizeof(syncPos); }
		ULONGLONG entityId = 0;
		Vector3 syncPos{};

	};

	// ���� �ֱ�� �ڽ��� Position ����
	struct UpdatePositionPacket : public PacketHeader
	{
		UpdatePositionPacket() { type = UPDATE_POSITION_PACKET; size = sizeof(timeStamp) + sizeof(entityId) + sizeof(pos); }
		ULONGLONG timeStamp = 0;
		ULONGLONG entityId = 0;
		Vector3 pos{};
	};
}

namespace C_Network
{
	// ---- HEART_BEAT
	struct HeartbeatPacket :public PacketHeader
	{
		HeartbeatPacket() { type = HEART_BEAT_PACKET; size = sizeof(timeStamp); }
		ULONGLONG timeStamp = 0;
	};
}

serializationBuffer& operator<< (serializationBuffer& serialBuffer, Vector3& vector);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, Vector3& vector);
// Only Has Head Packet
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::PacketHeader& packetHeader);

serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::ErrorPacket& errorPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::ErrorPacket& errorPacket);

// ���� ������ ��Ŷ�� ���� ������ �����ͱ����� �ֵ��� �Ѵ�.
// Packet ������ �� ��Ŷ�� �´� ����ȭ���� << operator�� ����������Ѵ�, PacketHeader�� ������ ����� �س��� ���¿����Ѵ�!  operator << >> �� �Է� / ��¸� ���� ���̴�.
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::EnterRoomResponsePacket& enterRoomResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket); 
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LeaveRoomNotifyPacket& leaveRoomNotifyPacket); 
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::OwnerChangeNotifyPacket& ownerChangeNotifyPacket); 
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::GameReadyNotifyPacket& gameReadyNotifyPacket);

// >> opeartor ����, >> operator�� PacketHeader�� ���� �и��� �����߱⿡ packetHeader�� �����ʹ� �Ű澲�� �ʾƵ� �ȴ�.
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::EnterRoomRequestPacket& enterRoomRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LeaveRoomRequestPacket& leaveRoomRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::OwnerChangeNotifyPacket& ownerChangeNotifyPacket);

