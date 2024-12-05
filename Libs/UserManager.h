#pragma once
#include "User.h"
#include <unordered_map>
namespace C_Network
{
	// UserManager�� ��ü�� �Ѱ��ϵ��� �Ѵ�. ���� Room�� �Ϻ� User�� ���� ������ �����ϱ� ������ UserManager�� ������� �ʵ��� �Ѵ�.
	class UserManager : public C_Utility::ManagerPool<User>
	{
	public:
		UserManager(uint maxUserCnt);
		~UserManager();

		User* AddUser(ULONGLONG userId,ULONGLONG sessionId); // �������� LoginResponsePacket�� �޾ƾ� userId�� ���� �� ����.
		ErrorCode DeleteUser(ULONGLONG userId);

		User* GetUserBySessionId(ULONGLONG sessionId); // �ش� session id�� ���� user�� ã�Ƴ��� �����Ѵ�.
		User* GetUserByUserId(ULONGLONG userId); // �ش� session id�� ���� user�� ã�Ƴ��� �����Ѵ�.

	private:

		SRWLOCK _sessionDicMapLock; // sessionId -> idx map lock
		SRWLOCK _userDicMapLock; // userId -> idx map lock

		// UserId -> SessionId�� user�� Login �� �� �ڽ��� session id�� ����� �� �ֵ��� �Ѵ�.
		// SessionId -> UserId;
		std::unordered_map<ULONGLONG, uint> _sessionToUserIdxMap;
		std::unordered_map<ULONGLONG, uint> _userIdToUserIdxMap;
	};
}
