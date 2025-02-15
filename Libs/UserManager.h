#pragma once
#include "User.h"
#include <unordered_map>
namespace C_Network
{
	// UserManager�� ��ü�� �Ѱ��ϵ��� �Ѵ�. ���� Room�� �Ϻ� User�� ���� ������ �����ϱ� ������ UserManager�� ������� �ʵ��� �Ѵ�.
	class UserManager 
	{
	public:
		UserManager(uint maxSessionCnt);
		~UserManager();

		SharedUser CreateUser(ULONGLONG userId, SharedSession sharedSession);// ULONGLONG sessionId); // �������� LoginResponsePacket�� �޾ƾ� userId�� ���� �� ����.
		ErrorCode DeleteUser(ULONGLONG userId);

		SharedUser GetUserBySessionId(ULONGLONG sessionId); // �ش� session id�� ���� user�� ã�Ƴ��� �����Ѵ�.
		SharedUser GetUserByUserId(ULONGLONG userId); // �ش� session id�� ���� user�� ã�Ƴ��� �����Ѵ�.

		uint GetUserCount() { return _sessionIdToUserMap.size(); }
	private:

		SRWLOCK _lock;

		// UserId -> SessionId�� user�� Login �� �� �ڽ��� session id�� ����� �� �ֵ��� �Ѵ�.
		// SessionId -> UserId;
		std::unordered_map<ULONGLONG, SharedUser> _sessionIdToUserMap;
		std::unordered_map<ULONGLONG, SharedUser> _userIdToUserMap;
	};
}
