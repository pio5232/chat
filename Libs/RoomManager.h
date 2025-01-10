#pragma once

#include "Room.h"
#include "CMonitor.h"

namespace C_Network
{
	class RoomManager
	{
	public:
		RoomManager(class ServerBase* owner, uint16 maxRoomCount, uint16 maxRoomUserCnt, class UserManager* userMgr);
		~RoomManager();

		//ErrorCode SendToAllUser(SharedSendBuffer& sendBuffer);
		//ErrorCode SendToRoom(uint16 roomNum, SharedSendBuffer& sendBuffer);
		ErrorCode SendToUserRoomInfo(ULONGLONG sessionId);
		
		SharedRoom CreateRoom(ULONGLONG ownerUserId, WCHAR* roomName);
		void DeleteRoom(uint16 roomNum);

		SharedRoom GetRoom(uint16 roomNum)
		{
			SRWLockGuard lockGuard(&_lock);

			auto iter = _roomMap.find(roomNum);
			if (iter == _roomMap.end())
			{
				TODO_LOG_ERROR;
				printf("Requested Room is Not exist, %u\n", roomNum);
				return nullptr;
			}

			return iter->second;
		}
	private:
		const uint16 _maxRoomCnt;
		const uint16 _maxRoomUserCnt; // room당 최대 user 수
		std::atomic<uint16> _roomCnt;

		std::unordered_map<uint16, SharedRoom> _roomMap; // [roomNum, roomPtr]
		SRWLOCK _lock;

		ServerBase* _owner;
		class UserManager* _userMgr;
	};
}