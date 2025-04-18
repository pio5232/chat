#pragma once

#include "JobQueue.h"
#include <set>

namespace C_Network
{
	// 현재는 Room을 모두 만들어놓고 (룸의 고유 번호는 증가되는 형태로) 사용할 것임.
	class Room : public C_Utility::JobQueue
	{
	public:
		enum class RoomState : byte
		{
			IDLE = 0, // 대기 중인 방.
			RUNNING, // 게임 중인 방.
		};

		Room(ULONGLONG ownerId, uint16 maxUserCnt, uint16 roomNum, WCHAR* roomName);
		~Room();

		void EnterRoom(LobbySessionPtr lobbySessionPtr);
		void LeaveRoom(LobbySessionPtr lobbySessionPtr);

		void ChatRoom(ULONGLONG sendUserId, SharedSendBuffer chatNotifyBuffer);
		uint16 GetCurUserCnt() const { return _userMap.size(); }
		uint16 GetMaxUserCnt() const { return _maxUserCnt; }
		uint16 GetRoomNum() const { return _roomNumber; }
		ULONGLONG GetOwnerId() const { return _ownerId; }
		void* GetRoomNamePtr() { return _roomName; }
		uint16 GetReadyCnt() { return _readyCnt; }
		void SetReady(LobbySessionPtr lobbySessionPtr, bool isReady, bool sendOpt);
		
		ErrorCode SendToAll(SharedSendBuffer sharedSendBuffer, ULONGLONG excludedId = 0, bool isexcluded = false);

		static int GetAliveRoomCount() { return _aliveRoomCount; }

	private:
		static std::atomic<int> _aliveRoomCount;

		std::unordered_map<ULONGLONG, std::weak_ptr<LobbySession>> _userMap; // USER ID - SESSION  

		ULONGLONG _ownerId = 0; // == userId;
		const uint16 _maxUserCnt = 0;
		uint16 _roomNumber = 0; // 몇 번째 방인지 확인한다.. 사용자가 CreateRoom하면 Room번호가 늘어나도록 만들자.
		WCHAR _roomName[ROOM_NAME_MAX_LEN] = {};

		RoomState _roomState = RoomState::IDLE;

		uint16 _readyCnt;
	};
}
