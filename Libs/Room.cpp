#include "LibsPch.h"
#include "Room.h"
#include "NetworkBase.h"

C_Network::Room::Room(C_Network::ServerBase* owner, uint16 maxUserCnt) :_owner(owner), _maxUserCnt(maxUserCnt), _curUserCnt(0)
{
	InitializeSRWLock(&_roomLock);
	
	_userList.reserve(maxUserCnt);
	
	Init();
}

void C_Network::Room::Init()
{
	// �� �κп� ���ؼ��� Room �ϳ��� ���ؼ� ��Ƽ������ ������ �������� üũ�غ�����.
	SRWLockGuard lockGuard(&_roomLock);
	
	static uint roomGenerator = 0;
	_ownerId = MAXULONGLONG;
	_curUserCnt = 0;
	_roomNumber = ++roomGenerator;
	_userList.clear();
}

void C_Network::Room::CreateRoom(ULONGLONG ownUserId)
{
	_ownerId = ownUserId;
	++_curUserCnt;
}

void C_Network::Room::EnterRoom(ULONGLONG userId)
{
	++_curUserCnt;
}

void C_Network::Room::LeaveRoom(ULONGLONG userId)
{
	++_curUserCnt;
}

C_Network::NetworkErrorCode C_Network::Room::SendToAll(SharedSendBuffer& sharedSendBuffer)
{
	for (User* userPtr : _userList)
	{
		_owner->Send(userPtr->GetSessionId(), sharedSendBuffer);
	}
	return C_Network::NetworkErrorCode::NONE;
}
