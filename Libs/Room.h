#pragma once

#include "User.h"
namespace C_Network
{
	// ����� Room�� ��� �������� (���� ���� ��ȣ�� �����Ǵ� ���·�) ����� ����.
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
		uint16 _roomNumber; // �� ��° ������ Ȯ���Ѵ�.. ����ڰ� CreateRoom�ϸ� Room��ȣ�� �þ���� ������.
		WCHAR _roomName[ROOM_NAME_MAX_LEN];

		std::vector<User*> _userList;
		
		class ServerBase* _owner;
	};
}
