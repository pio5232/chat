#pragma once


using serializationBuffer = C_Utility::CSerializationBuffer;

namespace C_Network
{
	const int ServerPort = 6000;

	const int MAX_PACKET_SIZE = 1000;
	TODO_DEFINITION;


	/// <summary>
	/// 편의성을 위해서 구조체를 정의하고 있지만, 나중이 되면 구조체는 삭제해야 한다.
	/// 원래는 직렬화 버퍼만 사용해서 복사가 Send / Recv 시 1번씩 일어났지만 현재는 구조체를 사용하기 때문에 복사가 2번씩 일어나고 있다.
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

		// 클라이언트의 요청
		CHAT_TO_ROOM_REQUEST_PACKET, // 방 안의 유저에게 메시지 전송 요청, RoomNum이 -1이면 모든 방에 전송.
		CHAT_TO_USER_REQUEST_PACKET, // 하나의 유저에게 메시지 전송 요청.
		CHAT_NOTIFY_PACKET,

		// 서버의 응답.
		CHAT_TO_ROOM_RESPONSE_PACKET,
		CHAT_TO_USER_RESPONSE_PACKET, // 

		HEART_BEAT_PACKET, // 연결 유지를 위한 패킷, 30초에 하나씩 보내도록 한다
		

		ECHO_PACKET = 65534,
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
	struct EchoPacket : public PacketHeader
	{
	public:
		EchoPacket() : data(0) { size = sizeof(__int64), type = ECHO_PACKET; }
		__int64 data;
	};

	// CHAT 

	struct ChatUserRequestPacket : public PacketHeader
	{
	public:
		//ULONGLONG sendUserId = 0;
		ULONGLONG targetUserId = 0;
		uint16 messageLen = 0;
		WCHAR payLoad[0];
		//WCHAR payLoad[MESSAGE_MAX_LEN];
	};
	struct ChatRoomRequestPacket : public PacketHeader
	{
		// TODO : 수정
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
		MakeRoomResponsePacket() { size = sizeof(isMade); type = MAKE_ROOM_RESPONSE_PACKET; }
		bool isMade = false;
	};

	struct alignas (64) EnterRoomResponsePacket : public PacketHeader
	{
		EnterRoomResponsePacket() { size = sizeof(bAllow) + RoomInfo::GetSize(); type = ENTER_ROOM_RESPONSE_PACKET; }
		bool bAllow = false;
		RoomInfo roomInfo = {};
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
		uint16 roomCnt;
		RoomInfo roomInfos[0];
	};	
}

// Only Has Head Packet
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::PacketHeader& packetHeader);

serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo);
;
// Packet 정의할 때 패킷에 맞는 직렬화버퍼 << operator를 정의해줘야한다, PacketHeader의 사이즈 계산은 해놓은 상태여야한다!  operator << >> 는 입력 / 출력만 행할 뿐이다.
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::EnterRoomResponsePacket& enterRoomResponsePacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket);

// >> opeartor 정의, >> operator는 PacketHeader에 대한 분리를 진행했기에 packetHeader의 데이터는 신경쓰지 않아도 된다.
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::EnterRoomResponsePacket& enterRoomResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket);

