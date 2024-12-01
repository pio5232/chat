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
		User();
		
		void InitInfo(ULONGLONG userId, ULONGLONG sessionId); // Initialize Info, TODO: 나중에 다른 정보가 .추가되어도 된다.

		ULONGLONG GetUserId() const { return _userId; }
		ULONGLONG GetSessionId() const { return _sessionId; }
	private:
		ULONGLONG _sessionId; // Session과 연결할 수 있는 수단. 이걸로 Network 작업을 요청한다.
		ULONGLONG _userId;
		WCHAR _nickName[USER_NAME_MAX_LEN]; // 유저 id
		uint _winCnt;
		uint _loseCnt;
	};
}
