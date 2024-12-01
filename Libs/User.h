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
		User();
		
		void InitInfo(ULONGLONG userId, ULONGLONG sessionId); // Initialize Info, TODO: ���߿� �ٸ� ������ .�߰��Ǿ �ȴ�.

		ULONGLONG GetUserId() const { return _userId; }
		ULONGLONG GetSessionId() const { return _sessionId; }
	private:
		ULONGLONG _sessionId; // Session�� ������ �� �ִ� ����. �̰ɷ� Network �۾��� ��û�Ѵ�.
		ULONGLONG _userId;
		WCHAR _nickName[USER_NAME_MAX_LEN]; // ���� id
		uint _winCnt;
		uint _loseCnt;
	};
}
