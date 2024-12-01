#pragma once
#include "User.h"
#include <unordered_map>
namespace C_Network
{
	// UserManager는 전체를 총괄하도록 한다. 따라서 Room은 일부 User에 관한 정보만 관리하기 때문에 UserManager를 사용하지 않도록 한다.
	class UserManager : public C_Utility::ManagerPool<User>
	{
	public:
		UserManager(uint maxUserCnt);
		~UserManager();

		User* AddUser(ULONGLONG userId,ULONGLONG sessionId); // 서버에서 LoginResponsePacket을 받아야 userId를 받을 수 있음.

		User* GetUser(ULONGLONG sessionId); // 해당 session id를 가진 user를 찾아내서 리턴한다.

	private:

		SRWLOCK _mapLock;
		// UserId -> SessionId는 user가 Login 할 때 자신의 session id를 등록할 수 있도록 한다.
		// SessionId -> UserId;
		std::unordered_map<ULONGLONG, uint> _sessionToUserIdxMap;
	};
}
