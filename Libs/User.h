#pragma once

namespace C_Network
{
	// Session - Network / User - Contents 영역.
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
				return 0; // 실패

			return _mySession.lock()->GetId(); 
		}

		void SetRoom(std::weak_ptr<Room> room) { _curRoom = room; }
		std::weak_ptr<Room> GetConnectedRoom() { return _curRoom; }
	private:
		std::weak_ptr<Session> _mySession;
		std::weak_ptr<Room> _curRoom;
		//ULONGLONG _sessionId; // Session과 연결할 수 있는 수단. 이걸로 Network 작업을 요청한다.
		ULONGLONG _userId;
		WCHAR _nickName[USER_NAME_MAX_LEN]; // 유저 id
		uint _winCnt;
		uint _loseCnt;
	};
}
