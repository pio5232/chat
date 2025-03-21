#pragma once

#include "Room.h"
#include "CMonitor.h"
#include "LobbyDefine.h"

namespace C_Network
{
	class RoomManager
	{
	private:
		RoomManager() : _maxRoomCnt(0), _maxRoomUserCnt(0) { InitializeSRWLock(&_lock); InitializeSRWLock(&_queueLock); }
	public:
		static RoomManager& GetInstance()
		{
			static RoomManager instance;
				
			return instance;
		}

		RoomManager(const RoomManager&) = delete;
		RoomManager& operator =(const RoomManager&) = delete;
		static void Init(uint16 maxRoomCount, uint16 maxRoomUserCnt);
		
		~RoomManager();

		ErrorCode SendToUserRoomInfo(LobbySessionPtr lobbySessionPtr);

		RoomPtr CreateRoom(LobbySessionPtr lobbySessionPtr, WCHAR* roomName);
		void DeleteRoom(uint16 roomNum);

		RoomPtr GetRoom(uint16 roomNum)
		{
			SRWLockGuard lockGuard(&_lock);

			auto iter = _roomMap.find(roomNum);
			if (iter == _roomMap.end())
			{
				printf("Requested Room is Not exist, %u\n", roomNum);
				return nullptr;
			}

			return iter->second;
		}

		void GetRoomsRead(OUT std::vector<std::weak_ptr<Room>>& roomVec)
		{
			SRWLockGuard lockGuard(&_lock);

			for (auto& pair : _roomMap)
			{
				roomVec.push_back(pair.second);
			}
		}

		void PushRoomNum(uint16 roomNum);
		uint16 PopRoomNum();

	private:
		uint16 _maxRoomCnt;
		uint16 _maxRoomUserCnt; // room당 최대 user 수
		std::atomic<uint16> _roomCnt;

		std::unordered_map<uint16, RoomPtr> _roomMap; // [roomNum, roomPtr]
		SRWLOCK _lock;
	private:
		SRWLOCK _queueLock;
		std::queue<uint16> _queue;
	};
}