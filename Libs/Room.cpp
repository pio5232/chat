#include "LibsPch.h"
#include "Room.h"
#include "NetworkBase.h"
#include "UserManager.h"
#include "ClientPacketHandler.h"

C_Network::Room::Room(C_Network::ServerBase* owner, UserManager* userMgr, class RoomManager* roomMgr, ULONGLONG ownerId, uint16 maxUserCnt, uint16 roomNum, WCHAR* roomName) : _owner(owner), _maxUserCnt(maxUserCnt), _userMgr(userMgr),
_ownerId(ownerId), _roomNumber(roomNum), _roomMgr(roomMgr), _readyCnt(0)
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
		SharedSendBuffer sendBuffer = ChattingClientPacketHandler::MakeErrorPacket(PacketErrorCode::FULL_ROOM);

		_owner->Send(sharedUser->GetSessionId(), sendBuffer);
		
		printf("EnterRoom Failed - RoomCnt is Full");

		return;
	}
	else if (_roomState == RoomState::RUNNING)
	{
		SharedSendBuffer sendBuffer = ChattingClientPacketHandler::MakeErrorPacket(PacketErrorCode::ALREADY_RUNNING_ROOM);

		_owner->Send(sharedUser->GetSessionId(), sendBuffer);
		
		printf("EnterRoom Failed - Room is Running");

		return;
	}

	sharedUser->SetRoom(std::static_pointer_cast<C_Network::Room>(shared_from_this()));

	responsePacket.bAllow = true;
	responsePacket.idCnt = 0;// _userMap.size();
	responsePacket.size += _userMap.size() * sizeof(ULONGLONG);

	{
		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakeSendBuffer(sizeof(responsePacket) + responsePacket.size);

		std::vector<ULONGLONG> tempVec;
		tempVec.reserve(_userMap.size());

		for (auto& pair : _userMap)
		{
			if (pair.second.expired())
				continue;

			SharedUser user = pair.second.lock();

			ULONGLONG idAndReadyState = user->GetUserId() | ((ULONGLONG)user->GetReady() << 63);

			responsePacket.idCnt++;

			tempVec.push_back(idAndReadyState);

			//*sendBuffer << pair.first;
		}
		*sendBuffer << responsePacket;

		for (ULONGLONG idAndReadyState : tempVec)
		{
			*sendBuffer << idAndReadyState;
		}

		_userMap.insert(std::make_pair(userId, user));

		SendToUser(sendBuffer, userId);
	}

	sharedUser->SetReady(false);

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
	bool isReady = false;
	if (!user.expired())
	{
		SharedUser sharedUser = user.lock();

		sharedUser->SetRoom(std::weak_ptr<C_Network::Room>()); 
		isReady = sharedUser->GetReady();
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

			SetReady(_ownerId, false, false);

			C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(ownerChangeNotifyPacket);

			SendToAll(sendBuffer);
		}
		else
		{
			if(isReady)
			_readyCnt--;
		}
	}
}

void C_Network::Room::ChatRoom(ULONGLONG sendUserId, SharedSendBuffer chatNotifyBuffer)
{
	SendToAll(chatNotifyBuffer);// , sendUserId, true);
}


void C_Network::Room::SetReady(ULONGLONG userId, bool isReady, bool sendOpt) // Ready 정보를 모두에게 알릴 것인가. (방장 교체의 경우 false => 알리지 않는다.)
{
	std::weak_ptr<User> user = _userMap[userId];
	if (user.expired())
	{
		printf("SetReady... User is Not Exist\n");
		return;
	}
	SharedUser sharedUser = user.lock();

	if (sharedUser->GetReady() == isReady)
		return;

	sharedUser->SetReady(isReady);

	if (isReady)
		_readyCnt++;
	else
		_readyCnt--;

	if (userId == _ownerId && _readyCnt == GetCurUserCnt())
	{
		C_Network::GameStartNotifyPacket startNotifyPacket;

		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(startNotifyPacket);

		SendToAll(sendBuffer);

		std::wstring path(L"E:\\GameServer\\x64\\Debug\\GameServer.exe");
		std::wstring args(std::to_wstring(_roomNumber) + L" " + std::to_wstring(GetCurUserCnt()) + L" " + std::to_wstring(GetMaxUserCnt()));
		wprintf(L"path : %s    args : %s", path, args);
		ExecuteProcess(path, args);
		return;
	}

	if (sendOpt)
	{
		C_Network::GameReadyNotifyPacket readyNotifyPacket;
		readyNotifyPacket.userId = userId;
		readyNotifyPacket.isReady = isReady;

		C_Network::SharedSendBuffer sendBuffer = C_Network::ChattingClientPacketHandler::MakePacket(readyNotifyPacket);

		SendToAll(sendBuffer);
	}
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

//C_Network::NetworkErrorCode C_Network::Room::SendGameServerInfo(SharedSendBuffer sharedSendBuffer)
//{
//	
//	return NetworkErrorCode();
//}

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
