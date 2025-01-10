#include "LibsPch.h"
#include "Room.h"
#include "NetworkBase.h"
#include "UserManager.h"

C_Network::Room::Room(C_Network::ServerBase* owner, UserManager* userMgr, ULONGLONG ownerId, uint16 maxUserCnt, uint16 roomNum, WCHAR* roomName) : _owner(owner), _maxUserCnt(maxUserCnt), _userMgr(userMgr),
_ownerId(ownerId), _roomNumber(roomNum)
{
	wmemcpy_s(_roomName, ROOM_NAME_MAX_LEN, roomName, ROOM_NAME_MAX_LEN);

	_userMap.reserve(maxUserCnt);

}

C_Network::Room::~Room()
{
}

void C_Network::Room::EnterRoom(ULONGLONG userId)
{
	std::weak_ptr<User> user = _userMgr->GetUserByUserId(userId);

	_userMap.insert(std::make_pair(userId, user));
}

void C_Network::Room::LeaveRoom(ULONGLONG userId)
{
	_userMap.erase(userId);
}


C_Network::NetworkErrorCode C_Network::Room::SendToAll(SharedSendBuffer& sharedSendBuffer)
{
	for (auto& pair : _userMap)
	{
		if(!pair.second.expired());
		{
			SharedUser user = pair.second.lock();
			
			_owner->Send(user->GetSessionId(), sharedSendBuffer);
		}
	}
	 
	return C_Network::NetworkErrorCode::NONE;
}
