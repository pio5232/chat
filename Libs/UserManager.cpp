#include "LibsPch.h"
#include "UserManager.h"

C_Network::UserManager::UserManager(uint maxUserCnt) : C_Utility::ManagerPool<User>(maxUserCnt)
{
	InitializeSRWLock(&_sessionDicMapLock);
	InitializeSRWLock(&_userDicMapLock);
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
		SRWLockGuard lockGuard(&_sessionDicMapLock);
		_sessionToUserIdxMap[sessionId] = idx;
	}
	{
		SRWLockGuard lockGuard(&_userDicMapLock);
		_userIdToUserIdxMap[userId] = idx;
	}
	InterlockedIncrement(&_curElementCnt);

	return newUser;// _elementArr[idx];
}

ErrorCode C_Network::UserManager::DeleteUser(ULONGLONG userId)
{
	uint arrIdx;
	{	
		SRWLockGuard lockGuard(&_userDicMapLock);

		auto userIter = _userIdToUserIdxMap.find(userId);
		if (userIter == _userIdToUserIdxMap.end())
			return ErrorCode::SESSION_USER_NOT_MAPPED;

		arrIdx = userIter->second; // index;
	
		_userIdToUserIdxMap.erase(userId);
	}

	ULONGLONG sessionId = _elementArr[arrIdx]->GetSessionId();
	{
		SRWLockGuard lockGuard(&_sessionDicMapLock);
		_sessionToUserIdxMap.erase(sessionId);
	}
	
	ObjectInitialize(arrIdx);

	wprintf(L"[%u User Delete. User id = %llu, \n", arrIdx, userId);

	return ErrorCode::NONE;
}


C_Network::User* C_Network::UserManager::GetUserBySessionId(ULONGLONG sessionId)
{
	SRWLockGuard lockGuard(&_sessionDicMapLock);

	auto userIter = _sessionToUserIdxMap.find(sessionId);
	if (userIter == _sessionToUserIdxMap.end())
		return nullptr;

	return _elementArr[userIter->second];
}

C_Network::User* C_Network::UserManager::GetUserByUserId(ULONGLONG userId)
{
	SRWLockGuard lockGuard(&_userDicMapLock);

	auto userIter = _userIdToUserIdxMap.find(userId);
	if (userIter == _userIdToUserIdxMap.end())
		return nullptr;

	return _elementArr[userIter->second];
}
