#include "LibsPch.h"
#include "UserManager.h"

C_Network::UserManager::UserManager(uint maxUserCnt) : C_Utility::ManagerPool<User>(maxUserCnt)
{
	InitializeSRWLock(&_mapLock);
}

C_Network::UserManager::~UserManager()
{
	C_Utility::ManagerPool<User>::~ManagerPool();
}

C_Network::User* C_Network::UserManager::AddUser(ULONGLONG userId, ULONGLONG sessionId)
{
	// session이 다 꽉 차면 받지 않으니까 ( user < session ) 이기 때문에 걱정하지 않는다.
	uint idx = GetAvailableIndex();

	C_Network::User* newUser = _elementArr[idx];

	newUser->InitInfo(userId, sessionId);

	{
		SRWLockGuard lockGuard(&_mapLock);
		_sessionToUserIdxMap[sessionId] = idx;
	}
	InterlockedIncrement(&_curElementCnt);

	return newUser;// _elementArr[idx];
}

C_Network::User* C_Network::UserManager::GetUser(ULONGLONG userId)
{
	SRWLockGuard lockGuard(&_mapLock);

	auto userIter = _sessionToUserIdxMap.find(userId);
	if (userIter == _sessionToUserIdxMap.end())
		return nullptr;

	return _elementArr[userIter->second];
}
