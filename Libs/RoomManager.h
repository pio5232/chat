#pragma once

#include "Room.h"

namespace C_Network
{
	class RoomManager : public C_Utility::ManagerPool<Room, class ServerBase* , uint16, class UserManager* >
	{
	public:
		RoomManager(class ServerBase* owner, uint16 maxRoomCount, uint16 maxRoomUserCnt, class UserManager* userMgr);
		~RoomManager();

		ErrorCode SendToAllUser(SharedSendBuffer& sendBuffer);
		ErrorCode SendToRoom(uint16 roomNum, SharedSendBuffer& sendBuffer);
		ErrorCode SendToUserRoomInfo(ULONGLONG sessionId, SharedSendBuffer& sendBuffer);
		
		Room* CreateRoom(uint16 ownerUserId, WCHAR* roomName);
	private:
		const uint16 _maxRoomUserCnt; // room당 최대 user 수
		std::unordered_map<uint16, Room*> _usingRoomDic; // [roomNum, roomPtr]
		SRWLOCK _roomDicLock;
		ServerBase* _owner;

		//class UserManager* _userMgr;
	};
}