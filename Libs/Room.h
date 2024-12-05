#pragma once

#include "User.h"
namespace C_Network
{
	// 현재는 Room을 모두 만들어놓고 (룸의 고유 번호는 증가되는 형태로) 사용할 것임.
	class Room
	{
	public :
		enum class RoomState : byte
		{
			RUNNING = 0, // 이미 생성되어서 실행중.
			IDLE, // 사용중인 상태가 아님.
		};

		Room(class ServerBase* owner, uint16 maxUserCnt, class UserManager* userMgr);

		void EnterRoom(ULONGLONG userId);
		void LeaveRoom(ULONGLONG userId);
		void InitRoomInfo(uint16 ownerUserId, WCHAR* roomName); // 초기화
		ErrorCode Close(); // 방 사라짐.

		uint16 GetCurUserCnt() const { return _curUserCnt; }
		uint16 GetMaxUserCnt() const { return _maxUserCnt; }
		uint16 GetRoomNum() const { return _roomNumber; }
		ULONGLONG GetOwnerId() const { return _ownerId; }
		const void* GetRoomNamePtr() { return _roomName; }

		NetworkErrorCode SendToAll(SharedSendBuffer& sharedSendBuffer);

		uint16 GenerateRoomNumber() {
			static volatile uint16 numGenerator = 0; return InterlockedIncrement16((volatile short*)&numGenerator);
		}
	private:

		SRWLOCK _roomLock;

		;ULONGLONG _ownerId = 0; // == userId;
		uint16 _curUserCnt = 0;
		const uint16 _maxUserCnt = 0;
		uint16 _roomNumber = 0; // 몇 번째 방인지 확인한다.. 사용자가 CreateRoom하면 Room번호가 늘어나도록 만들자.
		WCHAR _roomName[ROOM_NAME_MAX_LEN] = {};

		RoomState _roomState = RoomState::IDLE;
		std::vector<ULONGLONG> _userIdList;
		
		class UserManager* _userMgr = nullptr;
		class ServerBase* _owner;
	};
}
