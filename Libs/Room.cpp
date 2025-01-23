#include "LibsPch.h"
#include "Room.h"
#include "NetworkBase.h"
#include "UserManager.h"
#include "ClientPacketHandler.h"

C_Network::Room::Room(C_Network::ServerBase* owner, UserManager* userMgr, class RoomManager* roomMgr, ULONGLONG ownerId, uint16 maxUserCnt, uint16 roomNum, WCHAR* roomName) : _owner(owner), _maxUserCnt(maxUserCnt), _userMgr(userMgr),
_ownerId(ownerId), _roomNumber(roomNum), _roomMgr(roomMgr)
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

	if (user.expired())
		return;

	SharedUser sharedUser = user.lock();

	C_Network::EnterRoomResponsePacket responsePacket{};
	
	responsePacket.size = sizeof(responsePacket.bAllow) + sizeof(responsePacket.idCnt);
	if (_maxUserCnt == _userMap.size())
	{
		responsePacket.bAllow = false;
		responsePacket.idCnt = 0;

		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(responsePacket);

		_owner->Send(sharedUser->GetSessionId(), sendBuffer);

		printf("EnterRoom - RoomCnt is Full");
		return;
	}

	sharedUser->SetRoom(std::static_pointer_cast<C_Network::Room>(shared_from_this()));

	responsePacket.bAllow = true;
	responsePacket.idCnt = _userMap.size();
	responsePacket.size += _userMap.size() * sizeof(ULONGLONG);

	{
		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakeSendBuffer(sizeof(responsePacket) + responsePacket.size);

		*sendBuffer << responsePacket;
		for (auto& pair : _userMap)
		{
			*sendBuffer << pair.first;
		}

		_userMap.insert(std::make_pair(userId, user));

		SendToUser(sendBuffer, userId);
	}

	C_Network::EnterRoomNotifyPacket enterNotifyPacket;
	enterNotifyPacket.enterUserId = userId;
	{
		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(enterNotifyPacket);

		SendToAll(sendBuffer, userId, true);
	}
}

void C_Network::Room::LeaveRoom(ULONGLONG userId)
{
	{
		C_Network::LeaveRoomResponsePacket leaveRoomResponsePacket;
		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(leaveRoomResponsePacket);

		SendToUser(sendBuffer, userId);
	}

	std::weak_ptr<User> user = _userMgr->GetUserByUserId(userId);
	if (!user.expired())
	{
		SharedUser sharedUser = user.lock();

		sharedUser->SetRoom(std::weak_ptr<C_Network::Room>()); 
	}

	_userMap.erase(userId);
	{
		C_Network::LeaveRoomNotifyPacket leaveRoomNotifyPacket;
		leaveRoomNotifyPacket.leaveUserId = userId;
		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(leaveRoomNotifyPacket);

		SendToAll(sendBuffer);
	}
	
	if (_userMap.size() == 0)
	{
		// 마지막으로 남아있었다면 방 폭파.
		printf("방 폭파\n");
		_roomMgr->DeleteRoom(_roomNumber);
	}
	else
	{
		// 나갔는데 만약 방장이라면 방장을 넘겨준다.
		if (userId == _ownerId)
		{
			printf("방장이 나가서 위임\n");
			_ownerId = _userMap.begin()->first;
			
			C_Network::OwnerChangeNotifyPacket ownerChangeNotifyPacket;
			ownerChangeNotifyPacket.userId = _ownerId;

			C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(ownerChangeNotifyPacket);

			SendToAll(sendBuffer);
		}

	}
}

void C_Network::Room::ChatRoom(ULONGLONG sendUserId, SharedSendBuffer chatNotifyBuffer)
{
	SendToAll(chatNotifyBuffer);// , sendUserId, true);
}


C_Network::NetworkErrorCode C_Network::Room::SendToAll(SharedSendBuffer sharedSendBuffer, ULONGLONG excludedId, bool isexcluded)
{
	for (auto& pair : _userMap)
	{
		if(false == pair.second.expired())
		{
			SharedUser user = pair.second.lock();
			if (isexcluded && user->GetUserId() == excludedId)
				continue;

			_owner->Send(user->GetSessionId(), sharedSendBuffer);
		}
	}
	
	 
	return C_Network::NetworkErrorCode::NONE;
}

ErrorCode C_Network::Room::SendToUser(SharedSendBuffer sharedSendBuffer, ULONGLONG userId)
{
	auto iter = _userMap.find(userId);

	if (iter == _userMap.end())
	{
		printf("SendToUser - userId Not Found\n");
		return ErrorCode::NOT_FOUND;
	}
	if (iter->second.expired())
	{
		printf("SendToUser - weak_ptr is Expired\n");
		return ErrorCode::ACCESS_DELETE_MEMBER;
	}
	SharedUser sharedUser = iter->second.lock();

	_owner->Send(sharedUser->GetSessionId(), sharedSendBuffer);

	return ErrorCode::NONE;
}
