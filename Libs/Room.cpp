#include "LibsPch.h"
#include "Room.h"
#include "NetworkBase.h"
#include "UserManager.h"

C_Network::Room::Room(C_Network::ServerBase* owner, uint16 maxUserCnt, UserManager* userMgr) : _owner(owner), _maxUserCnt(maxUserCnt), _curUserCnt(1), _ownerId(0), _userMgr(userMgr)
{
	InitializeSRWLock(&_roomLock);
	
	_userIdList.reserve(maxUserCnt);

}

void C_Network::Room::EnterRoom(ULONGLONG userId)
{
	++_curUserCnt;
}

void C_Network::Room::LeaveRoom(ULONGLONG userId)
{
	--_curUserCnt;
}

void C_Network::Room::InitRoomInfo(uint16 ownerUserId, WCHAR* roomName) // 재사용 시초기화
{
	_ownerId = ownerUserId; // == userId;
	_curUserCnt = 1;
	_roomNumber = GenerateRoomNumber();
	_roomState = RoomState::RUNNING;
	wcscpy_s(_roomName, roomName);

	SRWLockGuard lockGuard(&_roomLock);
}

ErrorCode C_Network::Room::Close()
{
	_userIdList.clear();
	_roomState = RoomState::IDLE;
	return ErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::Room::SendToAll(SharedSendBuffer& sharedSendBuffer)
{
	for (ULONGLONG userId : _userIdList)
	{
		ULONGLONG sessionId = _userMgr->GetUserByUserId(userId)->GetSessionId();
		_owner->Send(sessionId, sharedSendBuffer);
	}
	return C_Network::NetworkErrorCode::NONE;
}
