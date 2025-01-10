#pragma once
#include "User.h"
#include <unordered_map>
namespace C_Network
{
	// UserManager는 전체를 총괄하도록 한다. 따라서 Room은 일부 User에 관한 정보만 관리하기 때문에 UserManager를 사용하지 않도록 한다.
	class UserManager 
	{
	public:
		UserManager(uint maxSessionCnt);
		~UserManager();

		SharedUser CreateUser(ULONGLONG userId, SharedSession sharedSession);// ULONGLONG sessionId); // 서버에서 LoginResponsePacket을 받아야 userId를 받을 수 있음.
		ErrorCode DeleteUser(ULONGLONG userId);

		SharedUser GetUserBySessionId(ULONGLONG sessionId); // 해당 session id를 가진 user를 찾아내서 리턴한다.
		SharedUser GetUserByUserId(ULONGLONG userId); // 해당 session id를 가진 user를 찾아내서 리턴한다.

	private:

		// TODO : 후에 READ_LOCK도 만든다면 속도의 향상을 기대할 수도 있을 것 같다.
		SRWLOCK _lock;

		// UserId -> SessionId는 user가 Login 할 때 자신의 session id를 등록할 수 있도록 한다.
		// SessionId -> UserId;
		std::unordered_map<ULONGLONG, SharedUser> _sessionIdToUserMap;
		std::unordered_map<ULONGLONG, SharedUser> _userIdToUserMap;
	};
}
