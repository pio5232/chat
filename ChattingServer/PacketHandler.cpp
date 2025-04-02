#include "pch.h"
#include "LobbyServer.h"
#include "LobbySession.h"
#include "PacketHandler.h"
#include "BufferMaker.h"
#include "RoomManager.h"
#include "LanSession.h"
/*---------------------------------------
			LobbyClientPacketHandler
---------------------------------------*/

std::unordered_map<uint16, C_Network::LobbyClientPacketHandler::PacketFunc> C_Network::LobbyClientPacketHandler::_packetFuncsDic;
std::unordered_map<uint16, C_Network::LanClientPacketHandler::PacketFunc> C_Network::LanClientPacketHandler::_packetFuncsDic;

void C_Network::LobbyClientPacketHandler::Init()
{
	_packetFuncsDic.clear();

	_packetFuncsDic[ROOM_LIST_REQUEST_PACKET] = ProcessRoomListRequestPacket;

	_packetFuncsDic[CHAT_TO_ROOM_REQUEST_PACKET] = ProcessChatToRoomPacket;
	_packetFuncsDic[CHAT_TO_USER_REQUEST_PACKET] = ProcessChatToUserPacket;

	_packetFuncsDic[LOG_IN_REQUEST_PACKET] = ProcessLogInPacket;
	_packetFuncsDic[MAKE_ROOM_REQUEST_PACKET] = ProcessMakeRoomRequestPacket;

	_packetFuncsDic[ENTER_ROOM_REQUEST_PACKET] = ProcessEnterRoomRequestPacket;
	_packetFuncsDic[LEAVE_ROOM_REQUEST_PACKET] = ProcessLeaveRoomRequestPacket;

	_packetFuncsDic[GAME_READY_REQUEST_PACKET] = ProcessGameReadyRequestPacket;

	_packetFuncsDic[HEART_BEAT_PACKET] = ProcessHeartbeatPacket;

}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessChatToRoomPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
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
	C_Network::SharedSendBuffer responseBuffer = C_Network::BufferMaker::MakePacket(packetHeader);
	lobbySessionPtr->Send(responseBuffer);

	ULONGLONG userId = lobbySessionPtr->_userId;

	// --- NotifyPacket
	packetHeader.size = sizeof(userId) + sizeof(messageLen) + messageLen;
	packetHeader.type = CHAT_NOTIFY_PACKET;

	C_Network::SharedSendBuffer notifyBuffer = C_Network::BufferMaker::MakeSendBuffer(sizeof(packetHeader) + packetHeader.size);

	*notifyBuffer << packetHeader << userId << messageLen;
	notifyBuffer->PutData(reinterpret_cast<const char*>(payLoad), messageLen);

	free(payLoad);

	RoomPtr roomPtr = RoomManager::GetInstance().GetRoom(roomNum);

	if (roomPtr == nullptr)
	{
		// LOG
		return ErrorCode::ACCESS_DESTROYED_ROOM;
	}
	//roomPtr->DoAsync(&Room::ChatRoom, userId, notifyBuffer);
	roomPtr->DoAsync(&Room::SendToAll, notifyBuffer, userId, false);

	return ErrorCode::NONE;
}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessChatToUserPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{

	return ErrorCode::NONE;
}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessMakeRoomRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	wprintf(L" MakeRoom Request Recv");

	WCHAR roomName[ROOM_NAME_MAX_LEN]{};

	buffer.GetData(reinterpret_cast<char*>(&roomName), sizeof(roomName));

	uint16 userId = lobbySessionPtr->_userId;

	RoomPtr roomPtr = RoomManager::GetInstance().CreateRoom(lobbySessionPtr, roomName);

	ErrorCode ret = ErrorCode::NONE;

	C_Network::MakeRoomResponsePacket makeRoomResponsePacket;

	if (roomPtr == nullptr)
	{
		makeRoomResponsePacket.isMade = false;

		printf("EnterRoom Response Packet Send -- [bAllow = false]");

		ret = ErrorCode::CREATE_ROOM_FAILED;
	}
	else
	{
		makeRoomResponsePacket.isMade = true;

		makeRoomResponsePacket.roomInfo.curUserCnt = roomPtr->GetCurUserCnt();
		makeRoomResponsePacket.roomInfo.maxUserCnt = roomPtr->GetMaxUserCnt();
		makeRoomResponsePacket.roomInfo.ownerId = roomPtr->GetOwnerId();
		makeRoomResponsePacket.roomInfo.roomNum = roomPtr->GetRoomNum();
		wmemcpy_s(makeRoomResponsePacket.roomInfo.roomName, ROOM_NAME_MAX_LEN, roomName, ROOM_NAME_MAX_LEN);
	}

	C_Network::SharedSendBuffer responsePacketBuffer = C_Network::BufferMaker::MakePacket(makeRoomResponsePacket);

	lobbySessionPtr->Send(responsePacketBuffer);

	return ret;
}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessEnterRoomRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	C_Network::EnterRoomRequestPacket requestPacket;

	buffer >> requestPacket;

	RoomPtr roomPtr = RoomManager::GetInstance().GetRoom(requestPacket.roomNum);

	if (nullptr == roomPtr)
	{
		SharedSendBuffer sendBuffer = C_Network::BufferMaker::MakeErrorPacket(PacketErrorCode::REQUEST_DESTROYED_ROOM);

		lobbySessionPtr->Send(sendBuffer);
		printf("Invalid Access - Destroyed Room");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	if (wcscmp(requestPacket.roomName, static_cast<const WCHAR*>(roomPtr->GetRoomNamePtr())) != 0)
	{
		SharedSendBuffer sendBuffer = C_Network::BufferMaker::MakeErrorPacket(PacketErrorCode::REQUEST_DIFF_ROOM_NAME);

		lobbySessionPtr->Send(sendBuffer);
		printf("EnterRoom - Room name is Diffrent\n");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	ULONGLONG userId = lobbySessionPtr->_userId;

	printf("EnterRoom Response Packet Send  ");

	roomPtr->DoAsync(&Room::EnterRoom, lobbySessionPtr);

	return ErrorCode::NONE;
}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessLeaveRoomRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	C_Network::LeaveRoomRequestPacket leaveRoomRequestPacket;

	buffer >> leaveRoomRequestPacket;

	RoomPtr roomPtr = RoomManager::GetInstance().GetRoom(leaveRoomRequestPacket.roomNum);

	if (roomPtr == nullptr)
	{
		// LOG
		return ErrorCode::ACCESS_DESTROYED_ROOM;
	}

	if (wcscmp(leaveRoomRequestPacket.roomName, static_cast<const WCHAR*>(roomPtr->GetRoomNamePtr())) != 0)
	{
		printf("LeaveRoom - Room name is Diffrent\n");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	ULONGLONG userId = lobbySessionPtr->_userId;

	roomPtr->DoAsync(&Room::LeaveRoom, lobbySessionPtr);

	return ErrorCode();
}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessGameReadyRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	C_Network::GameReadyRequestPacket requestPacket;

	buffer >> requestPacket.isReady;

	ULONGLONG userId = lobbySessionPtr->_userId;

	std::weak_ptr<Room> room = lobbySessionPtr->GetRoom();
	if (room.expired())
	{
		wprintf(L"ProcessGameReadyRequestPacket - Room Not Found");
		return ErrorCode::CANNOT_FIND_ROOM;
	}

	RoomPtr sharedRoom = room.lock();

	sharedRoom->DoAsync(&Room::SetReady, lobbySessionPtr, requestPacket.isReady, true);
}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessHeartbeatPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	ULONGLONG packetTimeStamp;

	buffer >> packetTimeStamp;
	printf("SessionId : %llu, [comTimeStamp : %llu], [PacketTimeStamp : %llu]\n", lobbySessionPtr->GetSessionId(), C_Utility::GetTimeStamp(), packetTimeStamp);
	
	lobbySessionPtr->UpdateHeartbeat(packetTimeStamp);

	return ErrorCode::NONE;
}


ErrorCode C_Network::LobbyClientPacketHandler::ProcessRoomListRequestPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	std::cout << "RoomList Request\n";

	ErrorCode errCode = RoomManager::GetInstance().SendToUserRoomInfo(lobbySessionPtr);

	return errCode;
}

ErrorCode C_Network::LobbyClientPacketHandler::ProcessLogInPacket(LobbySessionPtr& lobbySessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	std::cout << "LogIn Request\n";

	LogInRequestPacket clientRequestPacket;

	buffer >> clientRequestPacket;
	// 접속되어있는 녀석이 로그인을 누르면 무시.
	// 
	// ID와 비밀번호를 확인한 후 (클라이언트에서 암호화 -> 서버에서 복호화?)
	// 검증

	// 원래는 DB에서 USER 정보를 얻어와야 하지만, 현재는 DB 적용하지 않았기 때문에 3씩 증가시키도록 한다.
	static volatile ULONGLONG userIdGenerator = 4283;

	// AAA : DB에서 얻어오는걸로 변경.
	ULONGLONG userId = InterlockedAdd64((LONGLONG*)&userIdGenerator, 3);

	LogInResponsePacket clientResponsePacket;

	clientResponsePacket.size = sizeof(clientResponsePacket.userId);
	clientResponsePacket.type = LOG_IN_RESPONSE_PACKET;
	clientResponsePacket.userId = userId;

	lobbySessionPtr->_userId = userId;

	C_Network::SharedSendBuffer sendBuffer = C_Network::BufferMaker::MakePacket(clientResponsePacket);

	lobbySessionPtr->Send(sendBuffer);

	// 로그인 데이터를 DB에서 불러와서 설정 후 로그인 정보를

	// 해당 클라이언트에 전송한다...?

	// 현재는 일단 userId를 전송하도록 한다.

	return ErrorCode::NONE;
}

//ErrorCode C_Network::LanClientPacketHandler::ProcessLanInfoNotifyPacket(LobbySessionPtr& sharedSession, C_Utility::CSerializationBuffer& buffer)
//{
//	C_Network::GameServerLanInfoPacket packet;
//
//	buffer.GetData(reinterpret_cast<char*>(&packet.ipStr[0]), sizeof(packet.ipStr));
//
//	buffer >> packet.port >> packet.roomNum >> packet.xorToken;
//
//	wprintf(L"size : [ %d ]\n", packet.size);
//	wprintf(L"type : [ %d ]\n", packet.type);
//	wprintf(L"ip: [ %s ]\n", packet.ipStr);
//	wprintf(L"port : [ %d ]\n", packet.port);
//	wprintf(L"Room : [ %d ]\n", packet.roomNum);
//	wprintf(L"xorToken : [ %llu ], After : [%llu]\n", packet.xorToken, packet.xorToken ^ xorTokenKey);
//
//	C_Network::SharedSendBuffer sharedBuffer = LanClientPacketHandler::MakeSendBuffer(sizeof(packet));
//
//	*sharedBuffer << packet.size << packet.type;
//
//	sharedBuffer->PutData(reinterpret_cast<const char*>(packet.ipStr), IP_STRING_LEN * sizeof(WCHAR));
//
//	*sharedBuffer << packet.port << packet.roomNum << packet.xorToken;
//
//#ifdef LAN
//	_owner->ExecuteCallback(sharedBuffer, packet.roomNum, NetCallbackType::SEND_IPENDPOINT);
//#endif // LAN
//
//	return ErrorCode::NONE;
//}

void C_Network::LanClientPacketHandler::Init()
{
	_packetFuncsDic.clear();

	_packetFuncsDic[GAME_SERVER_LAN_INFO_PACKET] = ProcessLanInfoNotifyPacket; // ip Port
	_packetFuncsDic[GAME_SERVER_SETTING_REQUEST_PACKET] = ProcessGameSettingRequestPacket; // completePacket

}

ErrorCode C_Network::LanClientPacketHandler::ProcessLanInfoNotifyPacket(LanSessionPtr& lanSessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	printf("Lan Info Notify Packet Recv\n");
	C_Network::GameServerLanInfoPacket lanInfoPacket;
	
	buffer.GetData(reinterpret_cast<char*>(&lanInfoPacket.ipStr[0]), sizeof(lanInfoPacket.ipStr));

	buffer >> lanInfoPacket.port >> lanInfoPacket.roomNum >> lanInfoPacket.xorToken;

	wprintf(L"size : [ %d ]\n", lanInfoPacket.size);
	wprintf(L"type : [ %d ]\n", lanInfoPacket.type);
	wprintf(L"ip: [ %s ]\n", lanInfoPacket.ipStr);
	wprintf(L"port : [ %d ]\n", lanInfoPacket.port);
	wprintf(L"Room : [ %d ]\n", lanInfoPacket.roomNum);
	wprintf(L"xorToken : [ %llu ], After : [%llu]\n", lanInfoPacket.xorToken, lanInfoPacket.xorToken ^ xorTokenKey);

	C_Network::SharedSendBuffer sendBuffer = C_Network::BufferMaker::MakeSendBuffer(sizeof(lanInfoPacket));

	*sendBuffer << lanInfoPacket.size << lanInfoPacket.type;

	sendBuffer->PutData(reinterpret_cast<const char*>(lanInfoPacket.ipStr), IP_STRING_LEN * sizeof(WCHAR));

	*sendBuffer << lanInfoPacket.port << lanInfoPacket.roomNum << lanInfoPacket.xorToken;

	RoomPtr roomPtr = C_Network::RoomManager::GetInstance().GetRoom(lanInfoPacket.roomNum);

	if (roomPtr == nullptr)
	{
		// LOG
		return ErrorCode::ACCESS_DESTROYED_ROOM;
	}

	roomPtr->SendToAll(sendBuffer);

	return ErrorCode::NONE;
}

ErrorCode C_Network::LanClientPacketHandler::ProcessGameSettingRequestPacket(LanSessionPtr& lanSessionPtr, C_Utility::CSerializationBuffer& buffer)
{
	printf("Game Setting Request Packet Recv\n");
	uint16 roomNum = C_Network::RoomManager::GetInstance().PopRoomNum();
	
	RoomPtr roomPtr = C_Network::RoomManager::GetInstance().GetRoom(roomNum);

	if (roomPtr == nullptr)
	{
		// LOG
		printf("GameSetting.. Room is Invalid\n");
		return ErrorCode::ACCESS_DESTROYED_ROOM;
	}

	C_Network::GameServerSettingResponsePacket settingResponsePacket;

	settingResponsePacket.roomNum = roomNum;
	settingResponsePacket.requiredUsers = roomPtr->GetCurUserCnt();
	settingResponsePacket.maxUsers = roomPtr->GetMaxUserCnt();
	
	SharedSendBuffer sendBuffer = C_Network::BufferMaker::MakeSendBuffer(sizeof(settingResponsePacket));

	*sendBuffer << settingResponsePacket.size << settingResponsePacket.type << settingResponsePacket.roomNum << settingResponsePacket.requiredUsers << settingResponsePacket.maxUsers;
	
	lanSessionPtr->Send(sendBuffer);

	return ErrorCode::NONE;
}
