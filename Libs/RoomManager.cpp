#include "LibsPch.h"
#include "RoomManager.h"
#include "NetworkBase.h"
#include "UserManager.h"

C_Network::RoomManager::RoomManager(C_Network::ServerBase* owner, uint16 maxRoomCount, uint16 maxRoomUserCnt, UserManager* userMgr) : 
	C_Utility::ManagerPool<Room, ServerBase*, uint16, UserManager*>(maxRoomCount, owner, maxRoomUserCnt, userMgr),_owner(owner), _maxRoomUserCnt(maxRoomUserCnt)
{
	InitializeSRWLock(&_roomDicLock);

	_usingRoomDic.reserve(maxRoomCount);

	//for (int i = 0; i < maxRoomCount; i++)
	//{
	//	// TODO : ROOM POOL 사용
	//	Room* room = new Room(_owner, _maxRoomUserCnt, 0);

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

	{
		SRWLockGuard lockGuard(&_roomDicLock);
		
		for (auto& roomPair : _usingRoomDic)
		{
			Room* roomPtr = roomPair.second;

			*buffer << roomPtr->GetOwnerId() << roomPtr->GetRoomNum() << roomPtr->GetCurUserCnt() << roomPtr->GetMaxUserCnt();
			buffer->PutData(static_cast<const char*>(roomPtr->GetRoomNamePtr()), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);
		}
	}
	_owner->Send(sessionId, buffer);

	return ErrorCode::NONE;
}

C_Network::Room* C_Network::RoomManager::CreateRoom(uint16 ownerUserId, WCHAR* roomName)
{
	uint idx = GetAvailableIndex();

	if (idx == UINT_MAX)
	{
		//return ErrorCode::MAX_ROOM;
		C_Utility::Log(L" CreateRoom is Failed : MAX_ROOM");
		return nullptr;
	}
		
	C_Network::Room* newRoom = _elementArr[idx];

	newRoom->InitRoomInfo(ownerUserId, roomName);

	{
		SRWLockGuard lockGuard(&_roomDicLock);
		uint16 roomNum = newRoom->GetRoomNum();
		auto curRoom = _usingRoomDic.find(roomNum);// 존재하면 안된다.
		if (curRoom != _usingRoomDic.end())
		{
			C_Utility::Log(L"CreateRoom is Failed : Already Exist Room");
			return nullptr;
			//return ErrorCode::ALREADY_EXIST_ROOM;
		}
		_usingRoomDic[roomNum] = newRoom;
	}

	InterlockedIncrement(&_curElementCnt);

	return newRoom;
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
