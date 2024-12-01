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

		User* GetUser(ULONGLONG sessionId); // �ش� session id�� ���� user�� ã�Ƴ��� �����Ѵ�.

	private:

		SRWLOCK _mapLock;
		// UserId -> SessionId�� user�� Login �� �� �ڽ��� session id�� ����� �� �ֵ��� �Ѵ�.
		// SessionId -> UserId;
		std::unordered_map<ULONGLONG, uint> _sessionToUserIdxMap;
	};
}
