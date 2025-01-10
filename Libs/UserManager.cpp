#include "LibsPch.h"
#include "UserManager.h"

C_Network::UserManager::UserManager(uint maxSessionCnt) 
{
	InitializeSRWLock(&_lock);

	_sessionIdToUserMap.reserve(maxSessionCnt);
	_userIdToUserMap.reserve(maxSessionCnt);
}

C_Network::UserManager::~UserManager()
{
	
}

SharedUser C_Network::UserManager::CreateUser(ULONGLONG userId, SharedSession sharedSession) // ULONGLONG sessionId)
{
	// 어차피 maxSession보다 크면 session 레벨에서 끊을 것이기 때문에 제한 생각 x
	SharedUser sharedUser = std::make_shared<C_Network::User>(userId, sharedSession);

	ULONGLONG sessionId = sharedUser->GetSessionId();
	{
		SRWLockGuard lockGuard(&_lock);
		
		_sessionIdToUserMap.insert(std::make_pair(sessionId, sharedUser));
		_sessionIdToUserMap.insert(std::make_pair(userId, sharedUser));
	}

	return sharedUser;// _elementArr[idx];
}

ErrorCode C_Network::UserManager::DeleteUser(ULONGLONG userId)
{
	SRWLockGuard lockGuard(&_lock);
	
	auto userMapiter = _userIdToUserMap.find(userId);
	
	if (_userIdToUserMap.end() == userMapiter)
		return ErrorCode::SESSION_USER_NOT_MAPPED;

	ULONGLONG sessionId = userMapiter->second->GetSessionId();

	_userIdToUserMap.erase(userMapiter);

	auto sessionMapIter = _sessionIdToUserMap.find(sessionId);

	if (_sessionIdToUserMap.end() == sessionMapIter)
		return ErrorCode::SESSION_USER_NOT_MAPPED;

	_sessionIdToUserMap.erase(sessionMapIter);

	printf("[User Delete. User id = %llu, \n", userId);

	return ErrorCode::NONE;
}


SharedUser C_Network::UserManager::GetUserBySessionId(ULONGLONG sessionId)
{
	SRWSharedLockGuard lockGuard(&_lock);

	auto userIter = _sessionIdToUserMap.find(sessionId);
	if (userIter == _sessionIdToUserMap.end())
		return nullptr;

	return userIter->second;
}

SharedUser C_Network::UserManager::GetUserByUserId(ULONGLONG userId)
{
	SRWSharedLockGuard lockGuard(&_lock);

	auto userIter = _userIdToUserMap.find(userId);
	if (userIter == _userIdToUserMap.end())
		return nullptr;

	return userIter->second;
}
