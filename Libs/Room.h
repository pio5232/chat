#pragma once

#include "User.h"
namespace C_Network
{
	// 현재는 Room을 모두 만들어놓고 (룸의 고유 번호는 증가되는 형태로) 사용할 것임.
	class Room
	{
	public :
		Room(class ServerBase* owner, uint16 maxUserCnt);

		void Init();

		void CreateRoom(ULONGLONG ownUserId);

		void EnterRoom(ULONGLONG userId);
		void LeaveRoom(ULONGLONG userId);

		uint16 GetCurUserCnt() const { return _curUserCnt; }
		uint16 GetMaxUserCnt() const { return _maxUserCnt; }
		uint16 GetRoomNum() const { return _roomNumber; }
		ULONGLONG GetOwnerId() const { return _ownerId; }
		const void* GetRoomNamePtr() { return _roomName; }

		NetworkErrorCode SendToAll(SharedSendBuffer& sharedSendBuffer);

	private:
		SRWLOCK _roomLock;

		ULONGLONG _ownerId; // == userId;
		uint16 _curUserCnt;
		const uint16 _maxUserCnt;
		uint16 _roomNumber; // 몇 번째 방인지 확인한다.. 사용자가 CreateRoom하면 Room번호가 늘어나도록 만들자.
		WCHAR _roomName[ROOM_NAME_MAX_LEN];

		std::vector<User*> _userList;
		
		class ServerBase* _owner;
	};
}
