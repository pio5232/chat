#pragma once

#include "User.h"
#include "JobQueue.h"
#include <set>
namespace C_Network
{
	// ����� Room�� ��� �������� (���� ���� ��ȣ�� �����Ǵ� ���·�) ����� ����.
	class Room : public C_Utility::JobQueue
	{
	public :
		enum class RoomState : byte
		{
			IDLE = 0, // ��� ���� ��.
			RUNNING, // ���� ���� ��.
		};

		Room(class ServerBase* owner, class UserManager* userMgr,class RoomManager* roomMgr, ULONGLONG ownerId, uint16 maxUserCnt, uint16 roomNum, WCHAR* roomName);
		~Room();

		void EnterRoom(ULONGLONG userId);
		void LeaveRoom(ULONGLONG userId);

		void ChatRoom(ULONGLONG sendUserId, SharedSendBuffer chatNotifyBuffer);
		uint16 GetCurUserCnt() const { return _userMap.size(); }
		uint16 GetMaxUserCnt() const { return _maxUserCnt; }
		uint16 GetRoomNum() const { return _roomNumber; }
		ULONGLONG GetOwnerId() const { return _ownerId; }
		void* GetRoomNamePtr() { return _roomName; }


	private:
		NetworkErrorCode SendToAll(SharedSendBuffer sharedSendBuffer, ULONGLONG excludedId = 0, bool isexcluded = false);
		ErrorCode SendToUser(SharedSendBuffer sharedSendBuffer, ULONGLONG userId);

		std::unordered_map<ULONGLONG, std::weak_ptr<C_Network::User>> _userMap;

		ULONGLONG _ownerId = 0; // == userId;
		const uint16 _maxUserCnt = 0;
		uint16 _roomNumber = 0; // �� ��° ������ Ȯ���Ѵ�.. ����ڰ� CreateRoom�ϸ� Room��ȣ�� �þ���� ������.
		WCHAR _roomName[ROOM_NAME_MAX_LEN] = {};

		RoomState _roomState = RoomState::IDLE;
		
		class RoomManager* _roomMgr;
		class UserManager* _userMgr;
		class ServerBase* _owner;
	};
}
