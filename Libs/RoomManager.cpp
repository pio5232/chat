#include "LibsPch.h"
#include "RoomManager.h"
#include "NetworkBase.h"

C_Network::RoomManager::RoomManager(C_Network::ServerBase* owner, uint16 maxRoomCount, uint16 maxRoomUserCnt) : 
	C_Utility::ManagerPool<Room, ServerBase*, uint16 >(maxRoomCount, owner, maxRoomUserCnt),_owner(owner), _maxRoomUserCnt(maxRoomUserCnt)
{
	_usingRoomDic.reserve(maxRoomCount);

	//for (int i = 0; i < maxRoomCount; i++)
	//{
	//	// TODO : ROOM POOL »ç¿ë
	//	Room* room = new Room(_owner, _maxRoomUserCnt);

	//	_roomList.push_back(room);
	//}
}

C_Network::RoomManager::~RoomManager()
{}

ErrorCode C_Network::RoomManager::SendToUserRoomInfo(ULONGLONG sessionId, C_Network::SharedSendBuffer& buffer)
{
	// RoomListResponsePacket

	PacketHeader header;
	uint16 curRoomCnt = _usingRoomDic.size();//_curElementCnt;
	header.size = sizeof(curRoomCnt) + curRoomCnt * RoomInfo::GetSize();;
	header.type = ROOM_LIST_RESPONSE_PACKET;

	*buffer << header << curRoomCnt;

	for (auto& roomPair : _usingRoomDic)
	{
		Room* roomPtr = roomPair.second;

		*buffer << roomPtr->GetOwnerId() << roomPtr->GetRoomNum() << roomPtr->GetCurUserCnt() << roomPtr->GetMaxUserCnt();
		buffer->PutData(static_cast<const char*>(roomPtr->GetRoomNamePtr()), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);
	}

	_owner->Send(sessionId, buffer);

	return ErrorCode::NONE;
}

ErrorCode C_Network::RoomManager::SendToAllUser(SharedSendBuffer& sendBuffer)
{
	for (auto& pair : _usingRoomDic)
	{
		pair.second->SendToAll(sendBuffer);
	}

	return ErrorCode::NONE;
}

ErrorCode C_Network::RoomManager::SendToRoom(uint16 roomNum, SharedSendBuffer& sendBuffer)
{
	// find
	std::unordered_map<uint16, Room*>::iterator iter = _usingRoomDic.find(roomNum);

	if (iter == _usingRoomDic.end())
		return ErrorCode::CANNOT_FIND_ROOM;

	iter->second->SendToAll(sendBuffer);

	return ErrorCode::NONE;
}
