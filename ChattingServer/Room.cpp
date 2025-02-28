#include "pch.h"
#include "Room.h"
#include "NetworkBase.h"
#include "PacketHandler.h"
#include "PacketMaker.h"
#include "RoomManager.h"
#include "LobbySession.h"

C_Network::Room::Room(ULONGLONG ownerId, uint16 maxUserCnt, uint16 roomNum, WCHAR* roomName) : _maxUserCnt(maxUserCnt),
_ownerId(ownerId), _roomNumber(roomNum), _readyCnt(0)
{
	wmemcpy_s(_roomName, ROOM_NAME_MAX_LEN, roomName, ROOM_NAME_MAX_LEN);

	_userMap.reserve(maxUserCnt);

}

C_Network::Room::~Room()
{
	printf("방 폭파\n");

}

void C_Network::Room::EnterRoom(LobbySessionPtr lobbySessionPtr)
{
	C_Network::EnterRoomResponsePacket responsePacket{};

	responsePacket.size = sizeof(responsePacket.bAllow) + sizeof(responsePacket.idCnt);
	if (_maxUserCnt == _userMap.size())
	{
		SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakeErrorPacket(PacketErrorCode::FULL_ROOM);

		lobbySessionPtr->Send(sendBuffer);

		printf("EnterRoom Failed - RoomCnt is Full");

		return;
	}
	else if (_roomState == RoomState::RUNNING)
	{
		SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakeErrorPacket(PacketErrorCode::ALREADY_RUNNING_ROOM);

		lobbySessionPtr->Send(sendBuffer);

		printf("EnterRoom Failed - Room is Running");

		return;
	}

	lobbySessionPtr->SetRoom(std::static_pointer_cast<C_Network::Room>(shared_from_this()));

	responsePacket.bAllow = true;
	responsePacket.idCnt = 0;// _userMap.size();
	responsePacket.size += _userMap.size() * sizeof(ULONGLONG);

	ULONGLONG userId = lobbySessionPtr->_userId;

	{
		C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakeSendBuffer(sizeof(responsePacket) + responsePacket.size);

		std::vector<ULONGLONG> tempVec;
		tempVec.reserve(_userMap.size());

		for (auto& [id, weakLobbySessionPtr] : _userMap)
		{
			if (weakLobbySessionPtr.expired() == false)
			{
				LobbySessionPtr llobbySessionPtr = weakLobbySessionPtr.lock();

					ULONGLONG idAndReadyState = llobbySessionPtr->_userId | ((ULONGLONG)llobbySessionPtr->_isReady << 63);

				responsePacket.idCnt++;

				tempVec.push_back(idAndReadyState);
			}
			//*sendBuffer << pair.first;
		}
		*sendBuffer << responsePacket;

		for (ULONGLONG idAndReadyState : tempVec)
		{
			*sendBuffer << idAndReadyState;
		}

		_userMap.insert(std::make_pair(userId, lobbySessionPtr));

		lobbySessionPtr->Send(sendBuffer);
	}

	lobbySessionPtr->_isReady = false;

	C_Network::EnterRoomNotifyPacket enterNotifyPacket;
	enterNotifyPacket.enterUserId = userId;
	{
		C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakePacket(enterNotifyPacket);

		SendToAll(sendBuffer, userId, true);
	}
}

void C_Network::Room::LeaveRoom(LobbySessionPtr lobbySessionPtr)
{
	{
		C_Network::LeaveRoomResponsePacket leaveRoomResponsePacket;
		C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakePacket(leaveRoomResponsePacket);

		lobbySessionPtr->Send(sendBuffer);
	}

	bool isReady = false;

	ULONGLONG userId = lobbySessionPtr->_userId;

	lobbySessionPtr->SetRoom(std::weak_ptr<Room>());
	_userMap.erase(userId);
	{
		C_Network::LeaveRoomNotifyPacket leaveRoomNotifyPacket;
		leaveRoomNotifyPacket.leaveUserId = userId;
		C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakePacket(leaveRoomNotifyPacket);

		SendToAll(sendBuffer);
	}

	if (_userMap.size() == 0)
	{
		// 마지막으로 남아있었다면 방 폭파.
		RoomManager::GetInstance().DeleteRoom(_roomNumber);
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

			std::weak_ptr<LobbySession> weakLobbySessionPtr = _userMap[_ownerId];

			if(weakLobbySessionPtr.expired() == false)
			{
				LobbySessionPtr lobbySession = weakLobbySessionPtr.lock();

				SetReady(lobbySession, false, false);

				C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakePacket(ownerChangeNotifyPacket);

				SendToAll(sendBuffer);
			}
		}
		else
		{
			if (isReady)
				_readyCnt--;
		}
	}
}

void C_Network::Room::ChatRoom(ULONGLONG sendUserId, SharedSendBuffer chatNotifyBuffer)
{
	SendToAll(chatNotifyBuffer);// , sendUserId, true);
}


void C_Network::Room::SetReady(LobbySessionPtr lobbySessionPtr, bool isReady, bool sendOpt) // Ready 정보를 모두에게 알릴 것인가. (방장 교체의 경우 false => 알리지 않는다.)
{
	if (lobbySessionPtr->_isReady == isReady)
		return;

	lobbySessionPtr->_isReady = isReady;

	if (isReady)
		_readyCnt++;
	else
		_readyCnt--;

	ULONGLONG userId = lobbySessionPtr->_userId;

	if (userId == _ownerId && _readyCnt == GetCurUserCnt())
	{
		_roomState = RoomState::RUNNING;

		C_Network::GameStartNotifyPacket startNotifyPacket;

		C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakePacket(startNotifyPacket);

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

		C_Network::SharedSendBuffer sendBuffer = C_Network::PacketMaker::MakePacket(readyNotifyPacket);

		SendToAll(sendBuffer);
	}
}

ErrorCode C_Network::Room::SendToAll(SharedSendBuffer sharedSendBuffer, ULONGLONG excludedId, bool isexcluded)
{
	for (auto& [id, weakLobbySessionPtr] : _userMap)
	{
		if (weakLobbySessionPtr.expired())
			continue;
		
		LobbySessionPtr lobbySessionPtr = weakLobbySessionPtr.lock();

		if (isexcluded && lobbySessionPtr->_userId == excludedId)
			continue;

		lobbySessionPtr->Send(sharedSendBuffer);
		
	}

	return ErrorCode::NONE;
}

//C_Network::NetworkErrorCode C_Network::Room::SendGameServerInfo(SharedSendBuffer sharedSendBuffer)
//{
//	
//	return NetworkErrorCode();
//}
