#pragma once
using serializationBuffer = C_Utility::CSerializationBuffer;

namespace C_Network
{
	const int ServerPort = 6000;
	const int LanServerPort = 8768;

	const int MAX_PACKET_SIZE = 1000;
	const ULONGLONG HEARTBEAT_TIMEOUT = 30000; // MS, 30초

	// AAA 구성이나 로직이 동일한 패킷들은 합치도록?. (EX) EnterRoom / LeaveRoom Request Packet은 구성 요소가 똑같기 때문에 하나로 합치고 플래그만 조절하는 식으로 변경, (EnterRoom Notify / LeaveRoomNotify)

	/// <summary>
	/// 편의성을 위해서 구조체를 정의하고 있지만, 나중이 되면 구조체는 삭제해도됨..
	/// </summary>
	enum PacketType : uint16 // packet order
	{
		INVALID_PACKET = 65535,
		// C_S, REQUEST = 클라이언트 -> 서버
		// S_C, RESPONSE = 서버 -> 클라이언트
		LOG_IN_REQUEST_PACKET = 0, //
		LOG_IN_RESPONSE_PACKET,

		ROOM_LIST_REQUEST_PACKET,
		ROOM_LIST_RESPONSE_PACKET,

		MAKE_ROOM_REQUEST_PACKET,
		MAKE_ROOM_RESPONSE_PACKET, // -> ENTER_ROOM_RESPONSE_PACKET으로 RESPONSE를 보냄.

		ENTER_ROOM_REQUEST_PACKET,
		ENTER_ROOM_RESPONSE_PACKET,
		ENTER_ROOM_NOTIFY_PACKET,

		LEAVE_ROOM_REQUEST_PACKET,
		LEAVE_ROOM_RESPONSE_PACKET,
		LEAVE_ROOM_NOTIFY_PACKET,

		CLIENT_LOG_OUT_PACKET,
		CLIENT_LOG_OUT_NOTIFY_PACKET,

		OWNER_CHANGE_NOTIFY_PACKET,
		// 클라이언트의 요청
		CHAT_TO_ROOM_REQUEST_PACKET, // 방 안의 유저에게 메시지 전송 요청, RoomNum이 -1이면 모든 방에 전송.
		CHAT_TO_USER_REQUEST_PACKET, // 하나의 유저에게 메시지 전송 요청.
		CHAT_NOTIFY_PACKET,

		// 서버의 응답.
		CHAT_TO_ROOM_RESPONSE_PACKET,
		CHAT_TO_USER_RESPONSE_PACKET, // 

		GAME_READY_REQUEST_PACKET,
		//GAME_READY_RESPONSE_PACKET,
		GAME_READY_NOTIFY_PACKET,
		GAME_START_NOTIFY_PACKET,

		ERROR_PACKET, // 클라이언트에서 서버에 보냈지만 유효하지 않은 요청이 되어서 서버에서 사유와 함께 전달하도록 한다. ex) 삭제된 방에 들어가도록 하는 형태

		// -- LAN
		GAME_SERVER_SETTING_REQUEST_PACKET, // 1. 처음 GAME LOBBY에 접속하면 보내는 요청패킷.(GAME -> LOBBY)
		GAME_SERVER_SETTING_RESPONSE_PACKET, // 2. 게임 서버가 가져야 할 정보들을 보냄 (LOBBY -> GAME)
		GAME_SERVER_LAN_INFO_PACKET, // 3. 자신 ip,port 등 정보 보냄 (GAME -> LOBBY) 이거 받으면 방에 있는 아이들에게 바로 보냄.

		// -- GAME
		ENTER_GAME_REQUEST_PACKET,
		ENTER_GAME_RESPONSE_PACKET,
		LEAVE_GAME_NOTIFY_PACKET,

		MAKE_MY_CHARACTER_PACKET,
		MAKE_OTHER_CHARACTER_PACKET,

		GAME_LOAD_COMPELTE_PACKET, // MAKE PLAYER 가 다 작업이 되었다면.

		MOVE_START_REQUEST_PACKET,
		MOVE_START_NOTIFY_PACKET, // RESPONSE 겸 NOTIFY. 모든 유저에게 보냄
		MOVE_STOP_REQUEST_PACKET,
		MOVE_STOP_NOTIFY_PACKET, // RESPONSE 겸 NOTIFY. 모든 유저에게 보냄.

		CHARACTER_POSITION_SYNC_PACKET,
		UPDATE_POSITION_PACKET,

		HEART_BEAT_PACKET, // 연결 유지를 위한 패킷, 5초에 하나씩 보내도록 한다. // 30초가 지나면 연결이 끊긴 걸로 체크.

		ECHO_PACKET = 65534,
	};

	enum PacketErrorCode : uint16
	{
		REQUEST_DESTROYED_ROOM = 0, // 이미 삭제된 방에 진입 요청
		REQUEST_DIFF_ROOM_NAME, // 요청한 방 제목이 다르다.

		FULL_ROOM, // 인원이 꽉 찼다.
		ALREADY_RUNNING_ROOM, // 이미 게임이 시작된 방이다.

		// -- GAME
		CONNECTED_FAILED_WRONG_TOKEN, // 게임 서버에 연결할 때 토큰이 잘못된 토큰이다.
	};
	// 항상 padding이 존재하는지 확인해야한다.
	enum PacketSize
	{
		PACKET_SIZE_ECHO = 10,

		PACKET_SIZE_MAX = 500,
	};

	// size = payload의 size
	// 네트워크 코드에서 PacketHeader를 분리하게 되면 PacketHeader만큼의 사이즈는 사용하지 않는다.
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
	
	// head만 존재.
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
	// 암호화.. 복호화?
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
		ULONGLONG ids[0]; // 여기의 id 정보에는 최상위 bit가 Ready 상태를 가지도록 한다.

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
		uint16 requiredUsers = 0; // 로딩 유저 수
		uint16 maxUsers = 0; // Max, Max - required는 AI로 사용할 것.
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

	// 이후 GameStartPacket이 날아감.

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

	// 일정 주기로 자신의 Position 전달
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

// 가변 길이의 패킷은 고정 형태의 데이터까지만 넣도록 한다.
// Packet 정의할 때 패킷에 맞는 직렬화버퍼 << operator를 정의해줘야한다, PacketHeader의 사이즈 계산은 해놓은 상태여야한다!  operator << >> 는 입력 / 출력만 행할 뿐이다.
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::EnterRoomResponsePacket& enterRoomResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket); 
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LeaveRoomNotifyPacket& leaveRoomNotifyPacket); 
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::OwnerChangeNotifyPacket& ownerChangeNotifyPacket); 
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::GameReadyNotifyPacket& gameReadyNotifyPacket);

// >> opeartor 정의, >> operator는 PacketHeader에 대한 분리를 진행했기에 packetHeader의 데이터는 신경쓰지 않아도 된다.
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::EnterRoomRequestPacket& enterRoomRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LeaveRoomRequestPacket& leaveRoomRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::OwnerChangeNotifyPacket& ownerChangeNotifyPacket);

