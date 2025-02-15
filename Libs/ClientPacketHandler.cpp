#include "LibsPch.h"
#include "ClientPacketHandler.h"
#include "EchoServer.h"
#include "ChattingServer.h"
/*---------------------------------------
			ClientPacketHandler
---------------------------------------*/

ErrorCode C_Network::ChattingClientPacketHandler::ProcessChatToRoomPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	wprintf(L" ChatToRoom Request Recv");

	uint16 roomNum;
	uint16 messageLen;

	buffer >> roomNum >> messageLen;
	
	char* payLoad = static_cast<char*>(malloc(messageLen));
	
	buffer.GetData(payLoad, messageLen);

	PacketHeader packetHeader;

	packetHeader.size = 0;
	packetHeader.type = CHAT_TO_ROOM_RESPONSE_PACKET; 

	// --- ChatRoomResponsePacket
	C_Network::SharedSendBuffer responseBuffer = MakePacket(packetHeader);
	_owner->Send(sessionId, responseBuffer);

	SharedUser sharedUser = _userMgr->GetUserBySessionId(sessionId);

	if (!sharedUser)
	{
		wprintf(L"ProcEnterRoomRequestPacket - User Not Found");
		return ErrorCode::SESSION_USER_NOT_MAPPED;
	}

	ULONGLONG userId = sharedUser->GetUserId();

	// --- NotifyPacket
	packetHeader.size = sizeof(userId) + sizeof(messageLen) + messageLen;
	packetHeader.type = CHAT_NOTIFY_PACKET;

	C_Network::SharedSendBuffer notifyBuffer = MakeSendBuffer(sizeof(packetHeader) + packetHeader.size);
	
	*notifyBuffer << packetHeader << userId << messageLen;
	notifyBuffer->PutData(reinterpret_cast<const char*>(payLoad), messageLen);

	free(payLoad);

	SharedRoom sharedRoom = _roomMgr->GetRoom(roomNum);

	//sharedRoom->DoAsync(&Room::ChatRoom, userId, notifyBuffer);
	sharedRoom->DoAsync(&Room::SendToAll, notifyBuffer, userId, false);

	return ErrorCode::NONE;
}

ErrorCode C_Network::ChattingClientPacketHandler::ProcessChatToUserPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	//wprintf(L" ChatToUser Request Recv");

	//ULONGLONG targetSessionId = 0;
	//uint16 messageLen = 0;

	//buffer >> targetSessionId >> messageLen;

	//char* payLoad = static_cast<char*>(malloc(messageLen));

	//buffer.GetData(payLoad, messageLen);

	
	//// WCHAR* ~~~

	//PacketHeader packetHeader;

	//packetHeader.size = 0;
	//packetHeader.type = CHAT_TO_USER_RESPONSE_PACKET;

	//// --- ChatUserResponsePacket
	//C_Network::SharedSendBuffer responseBuffer = MakePacket(packetHeader);
	//_owner->Send(sessionId, responseBuffer);

	//// --- NotifyPacket
	//packetHeader.size = sizeof(sessionId) + sizeof(messageLen) + messageLen;
	//packetHeader.type = CHAT_NOTIFY_PACKET;

	//C_Network::SharedSendBuffer notifyBuffer = MakeSendBuffer(sizeof(packetHeader) + packetHeader.size);

	//*notifyBuffer << packetHeader << _userMgr->GetUserBySessionId(sessionId)->GetUserId() << messageLen;
	//notifyBuffer->PutData(reinterpret_cast<const char*>(payLoad), messageLen);

	//free(payLoad);

	//_owner->Send(targetSessionId, notifyBuffer);

	return ErrorCode::NONE;
}

ErrorCode C_Network::ChattingClientPacketHandler::ProcessMakeRoomRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	wprintf(L" MakeRoom Request Recv");

	WCHAR roomName[ROOM_NAME_MAX_LEN]{};

	buffer.GetData(reinterpret_cast<char*>(&roomName), sizeof(roomName));

	uint16 userId = _userMgr->GetUserBySessionId(sessionId)->GetUserId();

	SharedRoom newRoom = _roomMgr->CreateRoom(userId, roomName);
			
	ErrorCode ret = ErrorCode::NONE;

	C_Network::MakeRoomResponsePacket makeRoomResponsePacket;
	
	if (!newRoom)
	{
		makeRoomResponsePacket.isMade = false;

		printf("EnterRoom Response Packet Send -- [bAllow = false]");

		ret = ErrorCode::CREATE_ROOM_FAILED;
	}
	else
	{
		makeRoomResponsePacket.isMade = true;

		makeRoomResponsePacket.roomInfo.curUserCnt = newRoom->GetCurUserCnt();
		makeRoomResponsePacket.roomInfo.maxUserCnt = newRoom->GetMaxUserCnt();
		makeRoomResponsePacket.roomInfo.ownerId = newRoom->GetOwnerId();
		makeRoomResponsePacket.roomInfo.roomNum = newRoom->GetRoomNum();
		wmemcpy_s(makeRoomResponsePacket.roomInfo.roomName, ROOM_NAME_MAX_LEN, roomName, ROOM_NAME_MAX_LEN);
	}
	
	C_Network::SharedSendBuffer responsePacketBuffer = MakePacket(makeRoomResponsePacket); 

	_owner->Send(sessionId, responsePacketBuffer);
	
	return ret;
}

ErrorCode C_Network::ChattingClientPacketHandler::ProcessEnterRoomRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	C_Network::EnterRoomRequestPacket requestPacket;

	buffer >> requestPacket;

	SharedRoom sharedRoom = _roomMgr->GetRoom(requestPacket.roomNum);

	if (nullptr == sharedRoom)
	{
		SharedSendBuffer sendBuffer = MakeErrorPacket(PacketErrorCode::REQUEST_DESTROYED_ROOM);
		
		_owner->Send(sessionId, sendBuffer);
		printf("Invalid Access - Destroyed Room");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	if (wcscmp(requestPacket.roomName, static_cast<const WCHAR*>(sharedRoom->GetRoomNamePtr())) != 0)
	{
		SharedSendBuffer sendBuffer = MakeErrorPacket(PacketErrorCode::REQUEST_DIFF_ROOM_NAME);
		
		_owner->Send(sessionId, sendBuffer);
		printf("EnterRoom - Room name is Diffrent\n");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	SharedUser userPtr = _userMgr->GetUserBySessionId(sessionId);

	if (!userPtr)
	{
		wprintf(L"ProcEnterRoomRequestPacket - User Not Found");
		return ErrorCode::SESSION_USER_NOT_MAPPED;
	}
	printf("EnterRoom Response Packet Send  ");

	sharedRoom->DoAsync(&Room::EnterRoom, userPtr->GetUserId());

	return ErrorCode::NONE;
}

ErrorCode C_Network::ChattingClientPacketHandler::ProcessLeaveRoomRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	C_Network::LeaveRoomRequestPacket leaveRoomRequestPacket;

	buffer >> leaveRoomRequestPacket;

	SharedRoom sharedRoom = _roomMgr->GetRoom(leaveRoomRequestPacket.roomNum);

	if (wcscmp(leaveRoomRequestPacket.roomName, static_cast<const WCHAR*>(sharedRoom->GetRoomNamePtr())) != 0)
	{
		printf("LeaveRoom - Room name is Diffrent\n");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	SharedUser userPtr = _userMgr->GetUserBySessionId(sessionId);

	if (!userPtr)
	{
		wprintf(L"ProcLeaveRoomRequestPacket - User Not Found");
		return ErrorCode::SESSION_USER_NOT_MAPPED;
	}

	sharedRoom->DoAsync(&Room::LeaveRoom, userPtr->GetUserId());

	return ErrorCode();
}

ErrorCode C_Network::ChattingClientPacketHandler::ProcessGameReadyRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	C_Network::GameReadyRequestPacket requestPacket;

	buffer >> requestPacket.isReady;

	SharedUser sharedUser = _userMgr->GetUserBySessionId(sessionId);

	if (!sharedUser)
	{
		wprintf(L"ProcessGameReadyRequestPacket - User Not Found");
		return ErrorCode::SESSION_USER_NOT_MAPPED;
	}

	ULONGLONG userId = sharedUser->GetUserId();

	if (sharedUser->GetConnectedRoom().expired())
	{
		wprintf(L"ProcessGameReadyRequestPacket - Room Not Found");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	SharedRoom sharedRoom = sharedUser->GetConnectedRoom().lock();

	sharedRoom->DoAsync(&Room::SetReady, userId, requestPacket.isReady, true);

}

ErrorCode C_Network::ChattingClientPacketHandler::ProcessRoomListRequestPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	std::cout << "RoomList Request\n";

	SharedUser userPtr = _userMgr->GetUserBySessionId(sessionId);

	if (!userPtr)
	{
		wprintf(L"Room List Request Packet is Failed");
		return ErrorCode::SESSION_USER_NOT_MAPPED;
	}

	// AA
	ErrorCode errCode = _roomMgr->SendToUserRoomInfo(sessionId);

	return errCode;
}

ErrorCode C_Network::ChattingClientPacketHandler::ProcessLogInPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{ 
	std::cout << "LogIn Request\n";

	LogInRequestPacket clientRequestPacket;

	buffer >> clientRequestPacket;
	// ���ӵǾ��ִ� �༮�� �α����� ������ ����.
	// 
	// ID�� ��й�ȣ�� Ȯ���� �� (Ŭ���̾�Ʈ���� ��ȣȭ -> �������� ��ȣȭ?)
	// ����
	
	// ������ DB���� USER ������ ���;� ������, ����� DB �������� �ʾұ� ������ 3�� ������Ű���� �Ѵ�.
	static volatile ULONGLONG userIdGenerator = 4283;
	
	// AAA : DB���� �����°ɷ� ����.
	ULONGLONG userId = InterlockedAdd64((LONGLONG*)&userIdGenerator, 3);

	LogInResponsePacket clientResponsePacket;

	clientResponsePacket.size = sizeof(clientResponsePacket.userId);
	clientResponsePacket.type = LOG_IN_RESPONSE_PACKET;
	clientResponsePacket.userId = userId;
	
	SharedSession sharedSession = _sessionMgr->GetSession(sessionId);

	_userMgr->CreateUser(userId, sharedSession);

	C_Network::SharedSendBuffer sendBuffer = MakePacket(clientResponsePacket);

	_owner->Send(sessionId, sendBuffer);

	// �α��� �����͸� DB���� �ҷ��ͼ� ���� �� �α��� ������

	// �ش� Ŭ���̾�Ʈ�� �����Ѵ�...?

	// ����� �ϴ� userId�� �����ϵ��� �Ѵ�.
	
	return ErrorCode::NONE;
}

ErrorCode C_Network::LanClientPacketHandler::ProcessLanInfoNotifyPacket(ULONGLONG sessionId, C_Utility::CSerializationBuffer& buffer)
{
	C_Network::GameServerInfoNotifyPacket packet;
	
	buffer.GetData(reinterpret_cast<char*>(&packet.ipStr[0]), sizeof(packet.ipStr));

	buffer >> packet.port >> packet.roomNum >> packet.xorToken;

	wprintf(L"size : [ %d ]\n", packet.size);
	wprintf(L"type : [ %d ]\n", packet.type);
	wprintf(L"ip: [ %s ]\n", packet.ipStr);
	wprintf(L"port : [ %d ]\n", packet.port);
	wprintf(L"Room : [ %d ]\n", packet.roomNum);
	wprintf(L"xorToken : [ %llu ], After : [%llu]\n", packet.xorToken, packet.xorToken^xorTokenKey);

	C_Network::SharedSendBuffer sharedBuffer = LanClientPacketHandler::MakeSendBuffer(sizeof(packet));

	*sharedBuffer << packet.size << packet.type;

	sharedBuffer->PutData(reinterpret_cast<const char*>(packet.ipStr), IP_STRING_LEN * sizeof(WCHAR));

	*sharedBuffer << packet.port << packet.roomNum << packet.xorToken;

	_owner->ExecuteCallback(sharedBuffer, packet.roomNum, NetCallbackType::SEND_IPENDPOINT);

	return ErrorCode::NONE;
}
