#pragma once

#include "User.h"
namespace C_Network
{
	// ����� Room�� ��� �������� (���� ���� ��ȣ�� �����Ǵ� ���·�) ����� ����.
	class Room
	{
	public :
		enum class RoomState : byte
		{
			RUNNING = 0, // �̹� �����Ǿ ������.
			IDLE, // ������� ���°� �ƴ�.
		};

		Room(class ServerBase* owner, uint16 maxUserCnt, class UserManager* userMgr);

		void EnterRoom(ULONGLONG userId);
		void LeaveRoom(ULONGLONG userId);
		void InitRoomInfo(uint16 ownerUserId, WCHAR* roomName); // �ʱ�ȭ
		ErrorCode Close(); // �� �����.

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
		uint16 _roomNumber = 0; // �� ��° ������ Ȯ���Ѵ�.. ����ڰ� CreateRoom�ϸ� Room��ȣ�� �þ���� ������.
		WCHAR _roomName[ROOM_NAME_MAX_LEN] = {};

		RoomState _roomState = RoomState::IDLE;
		std::vector<ULONGLONG> _userIdList;
		
		class UserManager* _userMgr = nullptr;
		class ServerBase* _owner;
	};
}
