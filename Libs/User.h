#pragma once

namespace C_Network
{
	// Session - Network / User - Contents ����.
	class User
	{
	public:
		enum class UserState
		{
			MAIN,
			ROOM,
			IN_GAME,
		};
		User(ULONGLONG userId, SharedSession sharedSession);// ULONGLONG sessionId);
		

		ULONGLONG GetUserId() const { return _userId; }
		ULONGLONG GetSessionId() const 
		{ 
			if (_mySession.expired())
				return 0; // ����

			return _mySession.lock()->GetId(); 
		}

		void SetRoom(std::weak_ptr<Room> room) { _curRoom = room; }
		std::weak_ptr<Room> GetConnectedRoom() { return _curRoom; }
	private:
		std::weak_ptr<Session> _mySession;
		std::weak_ptr<Room> _curRoom;
		//ULONGLONG _sessionId; // Session�� ������ �� �ִ� ����. �̰ɷ� Network �۾��� ��û�Ѵ�.
		ULONGLONG _userId;
		WCHAR _nickName[USER_NAME_MAX_LEN]; // ���� id
		uint _winCnt;
		uint _loseCnt;
	};
}
