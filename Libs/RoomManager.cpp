#include "LibsPch.h"
#include "RoomManager.h"
#include "NetworkBase.h"
#include "UserManager.h"
#include "ClientPacketHandler.h"

C_Network::RoomManager::RoomManager(C_Network::ServerBase* owner, uint16 maxRoomCount, uint16 maxRoomUserCnt, UserManager* userMgr) : _owner(owner), _maxRoomCnt(maxRoomCount), _maxRoomUserCnt(maxRoomUserCnt), _userMgr(userMgr),_roomCnt(0)
{
	InitializeSRWLock(&_lock);

	_roomMap.reserve(maxRoomCount);

}

C_Network::RoomManager::~RoomManager()
{}

ErrorCode C_Network::RoomManager::SendToUserRoomInfo(ULONGLONG sessionId)
{
	// RoomListResponsePacket

	std::vector<std::weak_ptr<Room>> lazyProcBuf;

	{
		SRWLockGuard lockGuard(&_lock);

		for (auto& roomPair : _roomMap)
		{
			lazyProcBuf.push_back(roomPair.second);
		}
	}

	uint16 roomCnt = lazyProcBuf.size();

	PacketHeader header;
	header.size = sizeof(roomCnt) + roomCnt * RoomInfo::GetSize();;
	header.type = ROOM_LIST_RESPONSE_PACKET;
	
	C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakeSendBuffer(sizeof(RoomListResponsePacket) + roomCnt * sizeof(RoomInfo));

	*sendBuffer << header << roomCnt;

	for (std::weak_ptr<Room>& weakPtrRoom : lazyProcBuf)
	{
		if (false == weakPtrRoom.expired())
		{
			SharedRoom sharedRoom = weakPtrRoom.lock();

			*sendBuffer << sharedRoom->GetOwnerId() << sharedRoom->GetRoomNum() << sharedRoom->GetCurUserCnt() << sharedRoom->GetMaxUserCnt();
			sendBuffer->PutData(static_cast<const char*>(sharedRoom->GetRoomNamePtr()), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);
		}
	}

	_owner->Send(sessionId, sendBuffer);

	return ErrorCode::NONE;
}

SharedRoom C_Network::RoomManager::CreateRoom(ULONGLONG ownerUserId, WCHAR* roomName)
{
	if (_roomCnt.load() == _maxRoomCnt)
		return nullptr;

	_roomCnt.fetch_add(1);

	static volatile uint16 roomNumGen = 0;
	uint16 roomNum = InterlockedIncrement16((short*)&roomNumGen);

	SharedRoom sharedRoom = std::make_shared<C_Network::Room>(_owner, _userMgr, ownerUserId, _maxRoomUserCnt, roomNum, roomName);

	{
		SRWLockGuard lockGuard(&_lock);
		_roomMap.insert(std::pair(roomNum, sharedRoom));
	}
	sharedRoom->DoAsync(&Room::EnterRoom, ownerUserId);

	return sharedRoom;
}

void C_Network::RoomManager::DeleteRoom(uint16 roomNum)
{
	uint delSize = 0;
	{
		SRWLockGuard lockGuard(&_lock);
		delSize = _roomMap.erase(roomNum);
	}
	if (0 == delSize)
	{
		TODO_LOG_ERROR;
		printf("Delete Room is not exist.. %u\n", roomNum);
		return;
	}
}

//ErrorCode C_Network::RoomManager::SendToAllUser(SharedSendBuffer& sendBuffer)
//{
//	for (auto& pair : _roomMap)
//	{
//		pair.second->SendToAll(sendBuffer);
//	}
//
//	return ErrorCode::NONE;
//}

//ErrorCode C_Network::RoomManager::SendToRoom(uint16 roomNum, SharedSendBuffer& sendBuffer)
//{
//	// find
//	std::unordered_map<uint16, Room*>::iterator iter = _roomMap.find(roomNum);
//
//	if (iter == _roomMap.end())
//		return ErrorCode::CANNOT_FIND_ROOM;
//
//	iter->second->SendToAll(sendBuffer);
//
//	return ErrorCode::NONE;
//}
