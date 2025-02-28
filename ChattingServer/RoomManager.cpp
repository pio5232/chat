#include "pch.h"
#include "RoomManager.h"
#include "NetworkBase.h"
#include "PacketHandler.h"
#include "PacketMaker.h"
#include "LobbySession.h"

void C_Network::RoomManager::Init(uint16 maxRoomCount, uint16 maxRoomUserCnt)
{
	RoomManager& roomManager = GetInstance();

	roomManager._roomCnt = 0;
	roomManager._maxRoomCnt = maxRoomCount;
	roomManager._roomMap.reserve(maxRoomCount);
	roomManager._maxRoomUserCnt = maxRoomUserCnt;
}

C_Network::RoomManager::~RoomManager()
{
}

ErrorCode C_Network::RoomManager::SendToUserRoomInfo(LobbySessionPtr lobbySessionPtr)
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

	C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakeSendBuffer(sizeof(RoomListResponsePacket) + roomCnt * sizeof(RoomInfo));

	*sendBuffer << header << roomCnt;

	for (std::weak_ptr<Room>& weakPtrRoom : lazyProcBuf)
	{
		if (false == weakPtrRoom.expired())
		{
			RoomPtr sharedRoom = weakPtrRoom.lock();

			*sendBuffer << sharedRoom->GetOwnerId() << sharedRoom->GetRoomNum() << sharedRoom->GetCurUserCnt() << sharedRoom->GetMaxUserCnt();
			sendBuffer->PutData(static_cast<const char*>(sharedRoom->GetRoomNamePtr()), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);
		}
	}

	lobbySessionPtr->Send(sendBuffer);

	return ErrorCode::NONE;
}

RoomPtr C_Network::RoomManager::CreateRoom(LobbySessionPtr lobbySessionPtr, WCHAR* roomName)
{
	if (_roomCnt.load() == _maxRoomCnt)
		return nullptr;

	_roomCnt.fetch_add(1);

	static volatile uint16 roomNumGen = 0;
	uint16 roomNum = InterlockedIncrement16((short*)&roomNumGen);

	RoomPtr sharedRoom = std::make_shared<C_Network::Room>(lobbySessionPtr->_userId, _maxRoomUserCnt, roomNum, roomName);

	{
		SRWLockGuard lockGuard(&_lock);
		_roomMap.insert(std::pair(roomNum, sharedRoom));
	}
	sharedRoom->DoAsync(&Room::EnterRoom, lobbySessionPtr);

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
		printf("Delete Room is not exist.. %u\n", roomNum);
		return;
	}
}
