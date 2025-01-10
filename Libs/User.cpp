#include "LibsPch.h"
#include "User.h"

#include <sstream>

C_Network::User::User(ULONGLONG userId, SharedSession sharedSession) : _userId(userId), _winCnt(0),_loseCnt(0), _mySession(sharedSession) // , _sessionId(sessionId)
{
	static volatile ULONG nickGenerator = 1;

	ULONG genNum = InterlockedIncrement(&nickGenerator);

	std::wstringstream stream;

	stream << L"자동생성" << genNum;

	wcscpy_s(_nickName, stream.str().c_str());

}
