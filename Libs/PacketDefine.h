#pragma once


using serializationBuffer = C_Utility::CSerializationBuffer;

namespace C_Network
{
	const int ServerPort = 6000;

	const int MAX_PACKET_SIZE = 1000;
	TODO_DEFINITION;


	/// <summary>
	/// ���Ǽ��� ���ؼ� ����ü�� �����ϰ� ������, ������ �Ǹ� ����ü�� �����ؾ� �Ѵ�.
	/// ������ ����ȭ ���۸� ����ؼ� ���簡 Send / Recv �� 1���� �Ͼ���� ����� ����ü�� ����ϱ� ������ ���簡 2���� �Ͼ�� �ִ�.
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

		ENTER_ROOM_REQUEST_PACKET,
		ENTER_ROOM_RESPONSE_PACKET,
		ENTER_ROOM_NOTIFY_PACKET,

		LEAVE_ROOM_REQUEST_PACKET,
		LEAVE_ROOM_RESPONSE_PACKET,
		LEAVE_ROOM_NOTIFY_PACKET,

		CLIENT_LOG_OUT_PACKET,

		// Ŭ���̾�Ʈ�� ��û
		CHAT_TO_ROOM_REQUEST_PACKET, // �� ���� �������� �޽��� ���� ��û, RoomNum�� -1�̸� ��� �濡 ����.
		CHAT_TO_USER_REQUEST_PACKET, // �ϳ��� �������� �޽��� ���� ��û.
		CHAT_NOTIFY_PACKET,

		// ������ ����.
		CHAT_TO_ROOM_RESPONSE_PACKET,
		CHAT_TO_USER_RESPONSE_PACKET, // 

		HEART_BEAT_PACKET, // ���� ������ ���� ��Ŷ, 30�ʿ� �ϳ��� �������� �Ѵ�
		

		ECHO_PACKET = 65534,
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
		// TODO : ����
	public:
		uint16 roomNum = UINT16_MAX;
		uint16 messageLen = 0;
		WCHAR payLoad[MESSAGE_MAX_LEN];
	};
	
	// head�� ����.
	struct ChatUserResponsePacket : public PacketHeader
	{
	public:
		ChatUserResponsePacket(){ type = CHAT_TO_USER_RESPONSE_PACKET; }
	};
	
	struct ChatRoomResponsePacket : public PacketHeader 
	{
	public:
		ChatRoomResponsePacket() { type = CHAT_TO_ROOM_RESPONSE_PACKET; }
	};
	

	struct ChatOtherUserNotifyPacket : public PacketHeader
	{
	public :
		ULONGLONG sendUserId = 0;
		WCHAR payLoad[MESSAGE_MAX_LEN];
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
		ULONGLONG userId;
	};

	// LOG_OUT
	struct LogOutRequestPacket : public PacketHeader
	{

	};
	struct LogOutResponsePacket : public PacketHeader
	{

	};

	// ENTER_ROOM
	struct EnterRoomRequestPacket : public PacketHeader
	{
		uint16 roomNum = 0;
		WCHAR roomName[ROOM_NAME_MAX_LEN];
	};
	struct EnterRoomResponsePacket : public PacketHeader
	{

	};

	// LEAVE_ROOM
	struct LeaveRoomRequestPacket : public PacketHeader
	{
		uint16 roomNum = 0;
		WCHAR roomName[ROOM_NAME_MAX_LEN];
	};
	struct LeaveRoomResponsePacket : public PacketHeader
	{

	};
	struct RoomInfo
	{
		static uint16 GetSize() { return sizeof(ownerId) + sizeof(roomNum) + sizeof(curUserCnt) + sizeof(maxUserCnt) + sizeof(roomName); }
		ULONGLONG ownerId;
		uint16 roomNum;
		uint16 curUserCnt;
		uint16 maxUserCnt;
		WCHAR roomName[ROOM_NAME_MAX_LEN];
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

// Packet ������ �� ��Ŷ�� �´� ����ȭ���� << operator�� ����������Ѵ�, PacketHeader�� ������ ����� �س��� ���¿����Ѵ�!  operator << >> �� �Է� / ��¸� ���� ���̴�.
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);

// >> opeartor ����, >> operator�� PacketHeader�� ���� �и��� �����߱⿡ packetHeader�� �����ʹ� �Ű澲�� �ʾƵ� �ȴ�.
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket);
serializationBuffer& operator>> (serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo);
